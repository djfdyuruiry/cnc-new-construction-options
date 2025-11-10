#pragma once

/**
 * These classes provide an API for dynamically managing rules for a game, without
 * having to explicitly declare class members for each rule. It uses the built in
 * INIClass methods for loading and exporting rule values. Setting rule values after
 * loading is also supported.
 * 
 * Class hierarchy:
 * 
 *   - RuleSections --[contains many]--> RuleSection
 *   - IniRuleContext --[wrapper around]--> RuleSection + INIClass
 *   - RuleSection --[uses]--> IniRuleContext
 * 
 * Usage:
 * 
 *  It is intended that one RuleSections instance is added as a member of a RulesClass
 *  class. This way it can be used to load rules to/from INI files using INIClass instances. 
 */

#include <functional>
#include <optional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <variant>

#include <nlohmann/json.hpp>

#include "fixed.h"
#include "ini.h"
#include "logger.h"
#include "stringutils.h"

using json = nlohmann::json;

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned long ulong;

using RuleValueVariant = std::variant<int, bool, float, ushort, std::string, uint, char, uchar>;

template<typename T>
concept RuleValueVariantCompatible = (
    std::is_same_v<T, int> ||
    std::is_same_v<T, bool> ||
    std::is_same_v<T, float> ||
    std::is_same_v<T, ushort> ||
    std::is_same_v<T, uint> ||
    std::is_same_v<T, char>||
    std::is_same_v<T, uchar> ||
    std::is_same_v<T, std::string>
);

template<typename C, typename T>
concept TypeConverter = requires(std::string str, const std::string& str_ref, std::string_view str_view, const std::vector<T>& instances, const char c, T instance) {
    { C::template Get_Valid_Strings<T>() } -> std::same_as<std::vector<std::string>>;
    { C::template Get_Valid_Instances<T>() } -> std::same_as<std::vector<T>>;
    { C::template Try_Parse<T>(str) } -> std::same_as<std::optional<T>>;
    { C::template Try_Parse_Csv<T>(str_ref, c) } -> std::same_as<std::optional<std::vector<T>>>;
    { C::template Get_Default_Value<T>() } -> std::same_as<T>;
    { C::To_String(instance) } -> std::same_as<std::string>;
    { C::To_Csv_String(instances) } -> std::same_as<std::string>;
    { C::template Get_Type_Name<T>() } -> std::same_as<std::string_view>;
    { C::template Register_Rule_Type<T>(str_view, str_view) } -> std::same_as<void>;
    { C::template Register_Csv_Rule_Type<T>(str_view, str_view) } -> std::same_as<void>;
};

class RuleSection
{
public:
    const std::string SectionName;

    static bool Variants_Have_Same_Type(RuleValueVariant value_variant_a, RuleValueVariant value_variant_b);

    static std::string_view Get_Variant_Type(RuleValueVariant value_variant);

    static std::string Get_Variant_Values(RuleValueVariant value_variant);

    static std::string Variant_To_String(RuleValueVariant value_variant);

    RuleSection(
        std::string section_name,
        std::function<void(RuleSection&, std::string_view, const RuleValueVariant&)> on_rules_changed
    ) : SectionName(std::move(section_name)), OnRulesChanged(std::move(on_rules_changed)) {}

    template<typename T>
    RuleSection& With(INIClass& context, std::function<void(T&)> actions)
    {
        T contextWrapper = T(*this, context);

        actions(contextWrapper);

        return *this;
    }

    bool Has_Key(std::string_view name) const;

    std::vector<std::string_view> Rule_Names() const;

    RuleValueVariant Get_Variant(std::string_view name) const;

    std::optional<RuleValueVariant> Try_Get_Variant(std::string_view name) const;

    std::string_view Get_Type(std::string_view name) const;

    RuleSection& Set_Ini_Comment(INIClass& ini, const std::string& comment);

    template<RuleValueVariantCompatible T>
    RuleSection& Load_From_Ini(
        INIClass& ini,
        std::string_view name,
        T default_value,
        const std::optional<std::function<bool(std::string)>>& str_validator = std::nullopt
    )
    {
        auto sectionIsInIni = ini.Section_Present(SectionName.data());

        if (!sectionIsInIni) {
            CNC_LOGGER_DEBUG(
                "Loading default value '{}' for '{}', rule section not found in provided INI: [{}]",
                Variant_To_String(default_value),
                name,
                SectionName
            );

            Rules[name.data()] = default_value;
            return *this;
        }

        CNC_LOGGER_DEBUG(
            "Attempting to find rule in INI: [{}] -> {} (with default = {})",
            SectionName,
            name,
            Variant_To_String(default_value)
        );

        auto value = default_value;

        if constexpr (std::is_same_v<T, int>) {
            value = ini.Get_Int(SectionName.data(), name.data(), default_value);
        } else if constexpr (std::is_same_v<T, bool>) {
            value = ini.Get_Bool(SectionName.data(), name.data(), default_value);
        } else if constexpr (std::is_same_v<T, float>) {
            auto default_value_str = std::format("{}", default_value);

            Safe_Parse<float>(
                name,
                ini.Get_String(SectionName.data(), name.data(), default_value_str),
                value,
                ParseFloat
            );
        } else if constexpr (std::is_same_v<T, ushort>) {
            Safe_Parse_Int<ushort>(
                name,
                ini.Get_Int(SectionName.data(), name.data(), default_value),
                value,
                ValidateUShort
            );
        } else if constexpr (std::is_same_v<T, uint>) {
            auto default_value_str = std::format("{}", default_value);

            Safe_Parse<uint, ulong>(
                name,
                ini.Get_String(SectionName.data(), name.data(), default_value_str),
                value,
                ParseULong,
                ValidateUInt
            );
        } else if constexpr (std::is_same_v<T, char>) {
            Safe_Parse_Int<char>(
                name,
                ini.Get_Int(SectionName.data(), name.data(), default_value),
                value,
                ValidateChar
            );
        } else if constexpr (std::is_same_v<T, uchar>) {
            Safe_Parse_Int<uchar>(
                name,
                ini.Get_Int(SectionName.data(), name.data(), default_value),
                value,
                ValidateUChar
            );
        } else if constexpr (std::is_same_v<T, std::string>) {
            auto str_value = ini.Get_String(SectionName.data(), name.data(), default_value);

            // TODO: trim string to forgive spacing around rule string
            // forgive incorrect casing in rule values
            CncStringUtils::To_Upper(str_value);

            if (!str_validator.has_value() || str_validator.value()(str_value)) {
                value = str_value;
            }
        }

        CNC_LOGGER_DEBUG(
            "Imported rule from INI: [{}] -> {} = {}",
            SectionName,
            name,
            Variant_To_String(value)
        );

        Rules[name.data()] = value;

        return *this;
    }

    const RuleSection& Save_To_Ini(INIClass& ini, std::string_view name) const;

    void Save_All_To_Ini(INIClass& ini) const;

    template<RuleValueVariantCompatible T>
    [[nodiscard]]
    std::optional<T> Try_Get(std::string_view name) const
    {
        auto value_variant_optional = Try_Get_Variant(name);

        if (!value_variant_optional.has_value()) {
            return std::nullopt;
        }

        auto value_variant = value_variant_optional.value();

        if (!std::get_if<T>(&value_variant)) {
            CNC_LOGGER_FATAL(
                "Attempted to read rule using wrong type '{}' (correct type: {}), found in section: [{}] -> {}",
                typeid(T).name(),
                Get_Type(name),
                SectionName,
                name
            );
        }

        return *std::get_if<T>(&value_variant);
    }

    template<RuleValueVariantCompatible T>
    T Get(std::string_view name) const
    {
        auto value_optional = Try_Get<T>(name);

        if (!value_optional.has_value()) {
            CNC_LOGGER_FATAL("Game requested a rule that has not been loaded: [{}] -> {} | Rules cache didn't load correctly (Rule loading code bug?)", SectionName, name);
        }

        return value_optional.value();
    }

    RuleSection& Set(std::string_view name, RuleValueVariant value);

    template<RuleValueVariantCompatible T>
    const RuleSection& Get_With_Callback(std::string_view name, std::function<void(T)> callback) const
    {
        callback(Get<T>(name));

        return *this;
    }

    template<class T, TypeConverter<T> C>
    RuleSection& Set_Converter_Section_Type()
    {
        ConverterSectionTypeName = C::template Get_Type_Name<T>();

        return *this;
    }

    std::optional<std::string_view>& Get_Converter_Section_Type_Name();

    template<class T, TypeConverter<T> C>
    T Get_With_Converter(std::string_view name) const
    {
        auto rules_value = Get<std::string>(name);
        auto converted_value = C::template Try_Parse<T>(rules_value);

        if (!converted_value.has_value()) {
            CNC_LOGGER_FATAL(
                "Value '{}' stored for rule '[{}] -> {}' was unable to be converted back to original type: {}. Rules cache has been corrupted, as an invalid value was somehow loaded (validation code bug?)",
                rules_value,
                SectionName,
                name,
                typeid(T).name()
            );
        }

        return converted_value.value();
    }

    template<class T, TypeConverter<T> C>
    const RuleSection& Get_With_Converter_Callback(std::string_view name, std::function<void(T)> callback) const
    {
        callback(Get_With_Converter<T, C>(name));

        return *this;
    }

    template<class T, TypeConverter<T> C>
    std::vector<T> Get_With_Csv_Converter(std::string_view name) const
    {
        auto rules_value = Get<std::string>(name);
        auto converted_value = C::template Try_Parse_Csv<T>(rules_value);

        if (!converted_value.has_value()) {
            CNC_LOGGER_FATAL(
                "Value '{}' stored for rule '[{}] -> {}' was unable to be converted back to original type: list of {}. Rules cache has been corrupted, as an invalid value was somehow loaded (validation code bug?)",
                rules_value,
                SectionName,
                name,
                typeid(T).name()
            );
        }

        return converted_value.value();
    }

    template<class T, TypeConverter<T> C>
    const RuleSection& Get_With_Csv_Converter_Callback(
        std::string_view name,
        std::function<void(std::vector<T>)> callback
    ) const
    {
        callback(Get_With_Csv_Converter<T, C>(name));

        return *this;
    }

    template<class T, TypeConverter<T> C>
    RuleSection& Set_With_Converter(std::string_view name, T instance)
    {
        Set(name, C::To_String(instance));

        return *this;
    }

    template<class T, TypeConverter<T> C>
    RuleSection& Set_With_Converter(std::string_view name, std::string instance_string)
    {
        auto parsed_instance = C::template Try_Parse<T>(instance_string);

        if (!parsed_instance.has_value()) {
            auto type_strings = C::template Get_Valid_Strings<T>();

            throw std::invalid_argument(
                std::format(
                    "Failed to parse instance string '{}' as type: {} | valid_values={}",
                    instance_string,
                    C::template Get_Type_Name<T>(),
                    CncStringUtils::To_Csv(type_strings)
                )
            );
        }

        return Set(name, instance_string);
    }

    template<class T, TypeConverter<T> C>
    RuleSection& Set_With_Csv_Converter(std::string_view name, const std::vector<T>& instances)
    {
        return Set(name, C::To_Csv_String(instances));
    }

    template<class T, TypeConverter<T> C>
    RuleSection& Set_With_Csv_Converter(std::string_view name, std::string instances_csv)
    {
        auto parsed_instances = C::template Try_Parse_Csv<T>(instances_csv);

        if (!parsed_instances.has_value()) {
            auto type_strings = C::template Get_Valid_Strings<T>();

            throw std::invalid_argument(
                std::format(
                    "Failed to parse instance string '{}' as csv list of type: {} | valid_values={}",
                    instances_csv,
                    typeid(T).name(),
                    CncStringUtils::To_Csv(type_strings)
                )
            );
        }

        return Set(name, instances_csv);
    }

    friend void to_json(json& j, const RuleSection& p);
    friend void from_json(const json& j, RuleSection& p);
private:
    static inline const auto& Logger = CncLogger::For(RuleSection);

    static inline const std::function<float(const std::string&)> ParseFloat = [](const auto& s) { return std::stof(s); };
    static inline const std::function<ulong(const std::string&)> ParseULong = [](const auto& s) { return std::stoul(s); };

    static inline const std::function<bool(int)> ValidateUShort = [](auto v) { return v <= std::numeric_limits<ushort>::max(); };
    static inline const std::function<bool(ulong)> ValidateUInt = [](auto v) { return v <= std::numeric_limits<uint>::max(); };
    static inline const std::function<bool(int)> ValidateChar = [](auto v) { return v >= std::numeric_limits<char>::min() && v <= std::numeric_limits<char>::max(); };
    static inline const std::function<bool(int)> ValidateUChar = [](auto v) { return v >= std::numeric_limits<uchar>::min() && v <= std::numeric_limits<uchar>::max(); };

    std::map<std::string, RuleValueVariant> Rules;
    std::function<void(RuleSection&, std::string_view, const RuleValueVariant&)> OnRulesChanged;
    std::optional<std::string_view> ConverterSectionTypeName;

    template<class T, class U = T, class V = std::string>
    void Safe_Parse(
        std::string_view name,
        const V& source,
        T& target,
        std::function<U(const V&)> parse,
        std::function<bool(U)> validate = [](U _) { return true; }
    )
    {
        std::optional<U> parsed_value;
        std::string err;

        try {
            U result = parse(source);
            parsed_value = result;

            if (validate(result)) {
                target = static_cast<T>(result);
                return;
            }

            err = "validation_failure";
        } catch (const std::invalid_argument& _) {
            err = "invalid_argument";
        } catch (const std::out_of_range& _) {
            err = "out_of_range";
        }

        auto parse_msg = parsed_value.has_value() ? std::format(" | parsed_value={}", parsed_value.value()) : "";

        CNC_LOGGER_ERROR(
            "Invalid INI value '{}' for rule: [{}] -> {} | rule_type={} | valid_values={} | parse_error={}{}",
            source,
            SectionName,
            name,
            Get_Variant_Type(target),
            Get_Variant_Values(target),
            err,
            parse_msg
        );
    }

    template<class T>
    void Safe_Parse_Int(
        std::string_view name,
        const int& source,
        T& target,
        std::function<bool(int)> validate
    )
    {
        Safe_Parse<T, int, int>(
            name,
            source,
            target,
            [](const int& v) { return v; },
            validate
        );
    }
};

#define Read_Var_With_Type(VAR, T) Get_With_Callback<T>(#VAR, [&](const auto v) { VAR = v; })

// Load a variable/member by its C++ name from an INI context and set its value to equal the INI value
#define Read_Var(VAR) Read_Var_With_Type(#VAR, VAR, [&](const auto v) { VAR = v; })
#define Read_Bool_Var(VAR) Read_Var_With_Type(VAR, bool)
#define Read_UShort_Var(VAR) Read_Var_With_Type(VAR, ushort)
#define Read_Int_Var(VAR) Read_Var_With_Type(VAR, int)
#define Read_UInt_Var(VAR) Read_Var_With_Type(VAR, uint)
#define Read_Char_Var(VAR) Read_Var_With_Type(VAR, char)
#define Read_UChar_Var(VAR) Read_Var_With_Type(VAR, uchar)
#define Read_String_Var(VAR) Read_Var_With_Type(VAR, std::string)

class IniRuleContext
{
public:
    IniRuleContext(RuleSection& section, INIClass& context) : Section(section), Context(context) {}

    template<RuleValueVariantCompatible T>
    const IniRuleContext& Load(std::string_view name, T default_value) const
    {
        Section.Load_From_Ini(Context, name, default_value);

        return *this;
    }

    template<class T, TypeConverter<T> C>
    const IniRuleContext& Load_With_Converter(std::string_view name, T default_value) const
    {
        if (const auto type_name = Section.Get_Converter_Section_Type_Name(); type_name.has_value()) {
            C::template Register_Rule_Type<T>(type_name.value(), name);
        }

        Section.Load_From_Ini<std::string>(
            Context,
            name,
            C::To_String(default_value),
            [&](auto s) {
                auto is_valid = C::template Try_Parse<T>(s).has_value();

                if (!is_valid) {
                    auto type_strings = C::template Get_Valid_Strings<T>();

                    CNC_LOGGER_ERROR(
                         "Invalid INI value '{}' for rule: [{}] -> {} | rule_type={} | valid_values={} | parse_error=invalid_argument",
                         s,
                         Section.SectionName,
                         name,
                         typeid(T).name(),
                         CncStringUtils::To_Csv(type_strings)
                    );
                }

                return is_valid;
            }
        );

        return *this;
    }

    template<class T, TypeConverter<T> C>
    const IniRuleContext& Load_With_Csv_Converter(std::string_view name, const std::vector<T>& default_values) const
    {
        if (const auto type_name = Section.Get_Converter_Section_Type_Name(); type_name.has_value()) {
            C::template Register_Csv_Rule_Type<T>(type_name.value(), name);
        }

        Section.Load_From_Ini<std::string>(
            Context,
            name,
            C::To_Csv_String(default_values),
            [&](auto csv) {
                auto is_valid = C::template Try_Parse_Csv<T>(csv).has_value();

                if (!is_valid) {
                    CNC_LOGGER_ERROR(
                         "Invalid INI value '{}' for rule: [{}] -> {} | type=list of {}",
                         csv,
                         Section.SectionName,
                         name,
                         typeid(T).name()
                    );
                }

                return is_valid;
            }
        );

        return *this;
    }

    template<RuleValueVariantCompatible T>
    const IniRuleContext& Load_With_Callback(
        std::string_view name,
        T default_value,
        std::function<void(T)> callback
    ) const
    {
        Load(name, default_value);

        callback(Section.Get<T>(name));

        return *this;
    }

    template<class T, TypeConverter<T> C>
    const IniRuleContext& Load_With_Converter_Callback(
        std::string_view name,
        T default_value,
        std::function<void(T)> callback
    ) const
    {
        Load_With_Converter<T, C>(name, default_value);

        callback(Section.Get_With_Converter<T, C>(name));

        return *this;
    }

    template<class T, TypeConverter<T> C>
    const IniRuleContext& Load_With_Csv_Converter_Callback(
        std::string_view name,
        const std::vector<T>& default_values,
        std::function<void(std::vector<T>)> callback
    ) const
    {
        Load_With_Csv_Converter<T, C>(name, default_values);

        callback(Section.Get_With_Csv_Converter<T, C>(name));

        return *this;
    }

    const IniRuleContext& Save(std::string_view name) const;

    IniRuleContext& Load(std::string_view name);

    template<RuleValueVariantCompatible T>
    IniRuleContext& With_Default(T default_value)
    {
        if (!NameInStream.has_value()) {
            CNC_LOGGER_FATAL("Load(..) must be called before With_Default(..)");
        }

        Load(NameInStream.value(), default_value);

        NameInStream = std::nullopt;

        return *this;
    }

private:
    static inline const auto& Logger = CncLogger::For(IniRuleContext);

    RuleSection& Section;
    INIClass& Context;
    std::optional<std::string> NameInStream;
};

// IniRuleContext macro 'methods' - useful for setting variables and class members from rules

// Load a variable/member by its C++ name (with a specific type) from an INI context and set its value
// to equal the INI value
#define Load_Var_With_Type(VAR, T) Load_With_Callback<T>(#VAR, (T)VAR, [&](const auto v) { VAR = v; })

// Load a variable/member by its C++ name from an INI context and set its value to equal the INI value
#define Load_Var(VAR) Load_With_Callback(#VAR, VAR, [&](const auto v) { VAR = v; })
#define Load_Bool_Var(VAR) Load_Var_With_Type(VAR, bool)
#define Load_UShort_Var(VAR) Load_Var_With_Type(VAR, ushort)
#define Load_Int_Var(VAR) Load_Var_With_Type(VAR, int)
#define Load_UInt_Var(VAR) Load_Var_With_Type(VAR, uint)
#define Load_Char_Var(VAR) Load_Var_With_Type(VAR, char)
#define Load_UChar_Var(VAR) Load_Var_With_Type(VAR, uchar)
#define Load_String_Var(VAR) Load_Var_With_Type(VAR, std::string)

class RuleSections
{
public:
    void Default_Rules_Changed_Handler(std::function<void(RuleSection&, std::string_view, const RuleValueVariant&)> on_rules_changed);

    std::vector<std::string_view> Section_Names() const;

    bool Has_Section(std::string_view name) const;

    void Save_All_To_Ini(INIClass& ini) const;

    RuleSection& Add_Section(std::string_view name, std::function<void(RuleSection&, std::string_view, const RuleValueVariant&)> on_rules_changed);

    RuleSection& Add_Section(std::string_view name);

    RuleSection& operator[](std::string_view name);

private:
    static inline const auto& Logger = CncLogger::For(RuleSections);

    std::map<std::string, std::unique_ptr<RuleSection>> Sections;
    std::optional<std::function<void(RuleSection&, std::string_view, const RuleValueVariant&)>> OnRulesChangedDefault;
};
