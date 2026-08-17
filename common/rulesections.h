#pragma once

/**
 * These classes provide an API for dynamically managing rules for a game, without
 * having to explicitly declare class members for each rule. It uses the built-in
 * INIClass methods for loading and exporting rule values. Setting rule values after
 * loading is also supported. Supports converting string values to/from native game
 * engine values, given an appropriate TypeConverter implementation.
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
 *
 *  Can be used to manage non-rules INI files, for example in SettingsClass options for .
 */

#include <functional>
#include <optional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>

#include "ini.h"
#include "json.h"
#include "logger.h"
#include "paths.h"
#include "stringutils.h"

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned long ulong;

using RuleValueVariant = std::variant<int, bool, float, ushort, std::string, uint, char, uchar>;

TO_JSON(RuleValueVariant);
FROM_JSON(RuleValueVariant);

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
concept TypeConverter = requires(
    std::string str,
    const std::string& str_ref,
    std::string_view str_view,
    const std::vector<T>& instances,
    const char c, T instance
) {
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
    static bool Variants_Have_Same_Type(const RuleValueVariant& value_variant_a, const RuleValueVariant& value_variant_b);

    static std::string_view Get_Variant_Type(const RuleValueVariant& value_variant);

    static std::string Get_Variant_Values(const RuleValueVariant& value_variant);

    static std::string Variant_To_String(const RuleValueVariant& value_variant);

    const std::string& Get_Section_Name() const;

    RuleSection(
        std::string section_name,
        std::function<void(RuleSection&, std::string_view, const RuleValueVariant&)> on_rules_changed =
            [](const auto&, const auto, const auto&){}
    ) : SectionName(std::move(section_name)), SanitizeIniStrings(false), OnRulesChanged(std::move(on_rules_changed)) {}

    RuleSection& EnableStringSanitization();
    RuleSection& DisableStringSanitization();

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

    RuleSection& Set_Ini_Comment(const std::string& comment);

    const std::optional<std::string>& Get_Ini_Comment() const;

    template<RuleValueVariantCompatible T>
    RuleSection& Load_From_Ini(
        INIClass& ini,
        std::string_view name,
        T default_value,
        const std::optional<std::function<bool(std::string)>>& str_validator = std::nullopt
    )
    {
        // if entry has an existing value, keep it if parsing fails - apply default_value when no existing value found
        const auto entry_has_existing_value = Has_Key(name);
        const auto resolved_default_value = entry_has_existing_value ? Get<T>(name) : std::move(default_value);

        if (entry_has_existing_value) {
            CNC_LOGGER_DEBUG(
                "Existing value for rule detected: [{}] -> {} (value = {})",
                SectionName,
                name,
                resolved_default_value
            );
        }

        if (!ini.Section_Present(SectionName.data())) {
            CNC_LOGGER_DEBUG(
                "Loading value '{}' for '{}', rule section not found in provided INI: [{}]",
                resolved_default_value,
                name,
                SectionName
            );

            Rules[name.data()] = resolved_default_value;
            return *this;
        }

        CNC_LOGGER_DEBUG(
            "Attempting to find rule in INI: [{}] -> {} (with default = {})",
            SectionName,
            name,
            resolved_default_value
        );

        auto value = resolved_default_value;

        if constexpr (std::is_same_v<T, int>) {
            value = ini.Get_Int(SectionName.data(), name.data(), resolved_default_value);
        } else if constexpr (std::is_same_v<T, bool>) {
            value = ini.Get_Bool(SectionName.data(), name.data(), resolved_default_value);
        } else if constexpr (std::is_same_v<T, float>) {
            auto default_value_str = std::format("{}", resolved_default_value);

            Safe_Parse<float>(
                name,
                ini.Get_String(SectionName.data(), name.data(), default_value_str),
                value,
                ParseFloat
            );
        } else if constexpr (std::is_same_v<T, ushort>) {
            Safe_Parse_Int<ushort>(
                name,
                ini.Get_Int(SectionName.data(), name.data(), resolved_default_value),
                value,
                ValidateUShort
            );
        } else if constexpr (std::is_same_v<T, uint>) {
            auto default_value_str = std::format("{}", resolved_default_value);

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
                ini.Get_Int(SectionName.data(), name.data(), resolved_default_value),
                value,
                ValidateChar
            );
        } else if constexpr (std::is_same_v<T, uchar>) {
            Safe_Parse_Int<uchar>(
                name,
                ini.Get_Int(SectionName.data(), name.data(), resolved_default_value),
                value,
                ValidateUChar
            );
        } else if constexpr (std::is_same_v<T, std::string>) {
            auto str_value = ini.Get_String(SectionName.data(), name.data(), resolved_default_value);

            if (SanitizeIniStrings) {
                // TODO: trim string to forgive spacing around rule string
                // forgive incorrect casing in rule values
                CncStringUtils::To_Upper(str_value);
            }

            if (!str_validator.has_value() || str_validator.value()(str_value)) {
                value = str_value;
            }
        }

        // record where the value came from if we are setting an initial value or changing value
        if (
            (entry_has_existing_value && value != resolved_default_value) || (!entry_has_existing_value)
        ) {
            auto& filename = ini.Get_FileName();

            if (filename.has_value()) {
                auto filename_str = PathsClass::Get_Filename(filename->data());
                CncStringUtils::To_Upper(filename_str);

                RuleIniSource[name.data()] = filename_str;
            }
        }

        CNC_LOGGER_DEBUG(
            "Imported rule from INI: [{}] -> {} = {}",
            SectionName,
            name,
            value
        );

        Rules[name.data()] = value;

        return *this;
    }

    RuleSection& Parse_String(
        std::string_view name,
        const std::string& value_string,
        const RuleValueVariant& default_value,
        const std::optional<std::function<bool(std::string)>>& str_validator = std::nullopt
    );

    const RuleSection& Save_To_Ini(INIClass& ini, std::string_view name) const;

    void Save_All_To_Ini(INIClass& ini) const;

    template<RuleValueVariantCompatible T>
    [[nodiscard]]
    std::optional<T> Try_Get(std::string_view name) const
    {
        const auto value_variant_optional = Try_Get_Variant(name);

        if (!value_variant_optional.has_value()) {
            return std::nullopt;
        }

        const auto& value_variant = *value_variant_optional;

        if (!std::holds_alternative<T>(value_variant)) {
            static T default_value;

            CNC_LOGGER_FATAL(
                "Attempted to read rule using wrong type '{}' (correct type: {}), found in section: [{}] -> {}",
                Get_Variant_Type(default_value),
                Get_Variant_Type(value_variant),
                SectionName,
                name
            );
        }

        return std::get<T>(value_variant);
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

    /**
     * Get a string rule by name and write its value into a C String. Value is truncated automatically if provided
     * buffer is too small to store entire string.
     *
     * @param name Rule name
     * @param buffer Buffer to store rule value in
     * @param buffer_size Size of @param buffer
     * @return Number of characters in rule value string (excluding null terminator) - use this to detect if value was
     *         truncated when written to @param buffer (It will be bigger than @param buffer_size)
     */
    size_t Get_C_Str(std::string_view name, char* buffer, size_t buffer_size) const;

    RuleSection& Set(std::string_view name, RuleValueVariant value);
    RuleSection& Set(std::string_view name, const char* value);

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

    const std::optional<std::string>& Get_Converter_Section_Type_Name() const;

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
                C::template Get_Type_Name<T>()
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
                C::template Get_Type_Name<T>()
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

        CncStringUtils::To_Upper(instance_string);

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
                    C::template Get_Type_Name<T>(),
                    CncStringUtils::To_Csv(type_strings)
                )
            );
        }

        return Set(name, instances_csv);
    }

    RuleSection& Set_Rule_Comment(std::string_view name, std::string comment);
    std::optional<std::string> Try_Get_Rule_Comment(std::string_view name) const;

    RuleSection& Set_Rule_Ini_Source(std::string_view name, const std::string& source);
    std::optional<std::string> Try_Get_Rule_Ini_Source(std::string_view name) const;

    RuleValueVariant operator[](std::string_view name) const;

    // TODO: Handle OnRulesChanged, if needed
    JSON_FUNCTIONS(RuleSection)
private:
    static inline const auto& Logger = CncLogger::For(RuleSection);
    std::string SectionName;

    static inline const std::function<bool(const std::string&)> ParseBool = [](const auto& s) {
        auto potential_value = s;

        CncStringUtils::To_Lower(potential_value);

        if (potential_value == "true" || potential_value == "yes") {
            return true;
        }

        if (potential_value == "false" || potential_value == "no") {
            return false;
        }

        throw std::out_of_range(
            std::format("String '{}' could not be parsed as boolean value, true/yes/false/no expected.", s)
        );
    };
    static inline const std::function<int(const std::string&)> ParseInt = [](const auto& s) { return std::stoi(s); };
    static inline const std::function<float(const std::string&)> ParseFloat = [](const auto& s) { return std::stof(s); };
    static inline const std::function<ulong(const std::string&)> ParseULong = [](const auto& s) { return std::stoul(s); };

    static inline const std::function<bool(int)> ValidateUShort = [](auto v) { return v <= std::numeric_limits<ushort>::max(); };
    static inline const std::function<bool(ulong)> ValidateUInt = [](auto v) { return v <= std::numeric_limits<uint>::max(); };
    static inline const std::function<bool(int)> ValidateChar = [](auto v) {
        return v >= std::numeric_limits<char>::min() && v <= std::numeric_limits<char>::max();
    };
    static inline const std::function<bool(int)> ValidateUChar = [](auto v) {
        return v >= std::numeric_limits<uchar>::min() && v <= std::numeric_limits<uchar>::max();
    };

    /**
     * If true, when reading string rule values, the exact value from the INI file is used. If false, the
     * string is sanitized to forgive incorrect casing and whitespaces.
     */
    bool SanitizeIniStrings;
    std::optional<std::string> Comment;
    std::map<std::string, RuleValueVariant> Rules;
    std::map<std::string, std::string> RuleComments;
    std::map<std::string, std::string> RuleIniSource;
    std::function<void(RuleSection&, std::string_view, const RuleValueVariant&)> OnRulesChanged;
    std::optional<std::string> ConverterSectionTypeName;

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

    template<class T, class U = T, class V = std::string>
    void Parse(
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

        throw std::invalid_argument(
            std::format(
                "Parse error for value '{}': {}{}",
                source,
                err,
                parse_msg
            )
        );
    }

    template<class T>
    void Parse_Int(
        std::string_view name,
        const int& source,
        T& target,
        std::function<bool(int)> validate
    )
    {
        Parse<T, int, int>(
            name,
            source,
            target,
            [](const int& v) { return v; },
            validate
        );
    }
};

// RuleSection macro 'methods' - useful for setting variables and class members from rules

#define Read_Var_With_Type(VAR, T) Get_With_Callback<T>(#VAR, [&](const auto v) { VAR = v; })

// Load a variable/member by its C++ name from an INI context and set its value to equal the INI value
#define Read_Bool_Var(VAR) Read_Var_With_Type(VAR, bool)
#define Read_UShort_Var(VAR) Read_Var_With_Type(VAR, ushort)
#define Read_Int_Var(VAR) Read_Var_With_Type(VAR, int)
#define Read_UInt_Var(VAR) Read_Var_With_Type(VAR, uint)
#define Read_Char_Var(VAR) Read_Var_With_Type(VAR, char)
#define Read_UChar_Var(VAR) Read_Var_With_Type(VAR, uchar)
#define Read_String_Var(VAR) Read_Var_With_Type(VAR, std::string)

#define Read_Var_With_Name_And_Type(VAR, NAME, T) Get_With_Callback<T>(NAME, [&](const auto v) { VAR = v; })

// Load a variable/member by its C++ name from an INI context and set its value to equal the INI value
#define Read_Bool_Var_With_Name(VAR, NAME) Read_Var_With_Name_And_Type(VAR, NAME, bool)
#define Read_Negated_Bool_Var_With_Name(VAR, NAME) Get_With_Callback<bool>(NAME, [&](const auto v) { VAR = !v; })

#define Set_Var_Comment(VAR, COMMENT) Set_Rule_Comment(#VAR, COMMENT)

#define Set_Var_With_Type(VAR, T) Set(#VAR, (T)VAR)

#define Set_Bool_Var(VAR) Set_Var_With_Type(VAR, bool)
#define Set_UShort_Var(VAR) Set_Var_With_Type(VAR, ushort)
#define Set_Int_Var(VAR) Set_Var_With_Type(VAR, int)
#define Set_UInt_Var(VAR) Set_Var_With_Type(VAR, uint)
#define Set_Char_Var(VAR) Set_Var_With_Type(VAR, char)
#define Set_UChar_Var(VAR) Set_Var_With_Type(VAR, uchar)
#define Set_String_Var(VAR) Set_Var_With_Type(VAR, std::string)

#define Set_Var(VAR) Set(#VAR, VAR)

class IniRuleContext;

class IniRuleContext
{
public:
    IniRuleContext(RuleSection& section, INIClass& context) : Section(section), Context(context) {}

    template<RuleValueVariantCompatible T>
    IniRuleContext& Load(std::string_view name, T default_value)
    {
        Section.Load_From_Ini(Context, name, std::move(default_value));

        ValueInStream = Section.Get<T>(name);

        return *this;
    }

    template<class T, TypeConverter<T> C>
    IniRuleContext& Load_With_Converter(std::string_view name, T default_value)
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
                         Section.Get_Section_Name(),
                         name,
                         C::template Get_Type_Name<T>(),
                         CncStringUtils::To_Csv(type_strings)
                    );
                }

                return is_valid;
            }
        );

        ValueInStream = Section.Get<std::string>(name);

        return *this;
    }

    template<class T, TypeConverter<T> C>
    IniRuleContext& Load_With_Csv_Converter(std::string_view name, const std::vector<T>& default_values)
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
                         Section.Get_Section_Name(),
                         name,
                         C::template Get_Type_Name<T>()
                    );
                }

                return is_valid;
            }
        );

        ValueInStream = Section.Get<std::string>(name);

        return *this;
    }

    template<RuleValueVariantCompatible T>
    IniRuleContext& Load_With_Callback(
        std::string_view name,
        T default_value,
        std::function<void(T)> callback
    )
    {
        Load(name, default_value);

        callback(Section.Get<T>(name));

        return *this;
    }

    template<class T, TypeConverter<T> C>
    IniRuleContext& Load_With_Converter_Callback(
        std::string_view name,
        T default_value,
        std::function<void(T)> callback
    )
    {
        Load_With_Converter<T, C>(name, default_value);

        callback(Section.Get_With_Converter<T, C>(name));

        return *this;
    }

    template<class T, TypeConverter<T> C>
    IniRuleContext& Load_With_Csv_Converter_Callback(
        std::string_view name,
        const std::vector<T>& default_values,
        std::function<void(std::vector<T>)> callback
    )
    {
        Load_With_Csv_Converter<T, C>(name, default_values);

        callback(Section.Get_With_Csv_Converter<T, C>(name));

        return *this;
    }

    const IniRuleContext& Save(std::string_view name) const;

    IniRuleContext& Load(std::string_view name);

    IniRuleContext& With_Comment(std::string comment)
    {
        if (!NameInStream.has_value()) {
            CNC_LOGGER_FATAL("Load(..) must be called before With_Comment(..)");
        }

        Section.Set_Rule_Comment(NameInStream.value(), std::move(comment));

        return *this;
    }

    template<RuleValueVariantCompatible T>
    IniRuleContext& With_Default(T default_value)
    {
        if (!NameInStream.has_value()) {
            CNC_LOGGER_FATAL("Load(..) must be called before With_Default(..)");
        }

        Load(NameInStream.value(), std::move(default_value));

        return *this;
    }

    IniRuleContext& With_Default(const char* default_value)
    {
        return With_Default(std::string(default_value));
    }

    /**
     * Use bound_value as the default and then load the rule value from the INI context, writing the resolved value
     * back into bound_value.
     */
    template <RuleValueVariantCompatible T>
    IniRuleContext& With_Binding(T& bound_value)
    {
        With_Default<T>(bound_value);

        bound_value = Section.Get<T>(NameInStream.value());

        return *this;
    }

    template<RuleValueVariantCompatible T>
    IniRuleContext& Then_Set(T& ref)
    {
        if (!NameInStream.has_value() || !ValueInStream.has_value()) {
            CNC_LOGGER_FATAL("Load(..) must be called before And_Then_Set(..)");
        }

        ref = std::get<T>(*ValueInStream);

        return *this;
    }

    template<class T, TypeConverter<T> C>
    IniRuleContext& Then_Set_With_Converter(T& ref)
    {
        if (!NameInStream.has_value() || !ValueInStream.has_value()) {
            CNC_LOGGER_FATAL("Load(..) must be called before And_Then_Set(..)");
        }

        ref = C::template Assert_Parse<T>(
            std::get<std::string>(*ValueInStream),
            "IniRuleContext::ValueInStream"
        );

        return *this;
    }

    template<RuleValueVariantCompatible T, class U>
    IniRuleContext& Then_Set_With_Type(U& ref)
    {
        if (!NameInStream.has_value()) {
            CNC_LOGGER_FATAL("Load(..) must be called before Then_Set_With_Type(..)");
        }

        ref = static_cast<U>(std::get<T>(*ValueInStream));

        return *this;
    }

    template<RuleValueVariantCompatible T>
    IniRuleContext& With_Callback(const std::function<void(T)> callback)
    {
        if (!NameInStream.has_value()) {
            CNC_LOGGER_FATAL("Load(..) must be called before With_Callback(..)");
        }

        callback(std::get<T>(*ValueInStream));

        return *this;
    }

    RuleSection& Get_Section() const
    {
        return Section;
    }

private:
    static inline const auto& Logger = CncLogger::For(IniRuleContext);

    RuleSection& Section;
    INIClass& Context;
    std::optional<std::string> NameInStream;
    std::optional<RuleValueVariant> ValueInStream;
};

// IniRuleContext macro 'methods' - useful for setting variables and class members from rules

// Load a variable/member by its C++ name (with a specific type) from an INI context and set its value
// to equal the INI value
#define Load_Var_With_Type(VAR, T, DEFAULT) template Load_With_Callback<T>(#VAR, (T)DEFAULT, [&](const T v) { VAR = v; })

// Load a variable/member by its C++ name from an INI context and set its value to equal the INI value
#define Load_Var(VAR) Load_With_Callback(#VAR, VAR, [&](const auto v) { VAR = v; })

#define Load_Bool_Var_With_Default(VAR, DEFAULT) Load_Var_With_Type(VAR, bool, DEFAULT)
#define Load_UShort_Var_With_Default(VAR, DEFAULT) Load_Var_With_Type(VAR, ushort, DEFAULT)
#define Load_Int_Var_With_Default(VAR, DEFAULT) Load_Var_With_Type(VAR, int, DEFAULT)
#define Load_UInt_Var_With_Default(VAR, DEFAULT) Load_Var_With_Type(VAR, uint, DEFAULT)
#define Load_Char_Var_With_Default(VAR, DEFAULT) Load_Var_With_Type(VAR, char, DEFAULT)
#define Load_UChar_Var_With_Default(VAR, DEFAULT) Load_Var_With_Type(VAR, uchar, DEFAULT)
#define Load_String_Var_With_Default(VAR, DEFAULT) Load_Var_With_Type(VAR, std::string, DEFAULT)

#define Load_Bool_Var(VAR) Load_Var_With_Type(VAR, bool, VAR)
#define Load_UShort_Var(VAR) Load_Var_With_Type(VAR, ushort, VAR)
#define Load_Int_Var(VAR) Load_Var_With_Type(VAR, int, VAR)
#define Load_UInt_Var(VAR) Load_Var_With_Type(VAR, uint, VAR)
#define Load_Char_Var(VAR) Load_Var_With_Type(VAR, char, VAR)
#define Load_UChar_Var(VAR) Load_Var_With_Type(VAR, uchar, VAR)
#define Load_String_Var(VAR) Load_Var_With_Type(VAR, std::string, VAR)
#define Load_With_Method_Call(VAR, METHOD) Load_With_Callback(#VAR, VAR, [&](const auto v) { METHOD(v); })

class RuleSections
{
public:
    void Default_Rules_Changed_Handler(std::function<void(RuleSection&, std::string_view, const RuleValueVariant&)> on_rules_changed);

    std::vector<std::string_view> Section_Names() const;

    bool Has_Section(std::string_view name) const;

    void Save_All_To_Ini(INIClass& ini) const;

    void Save_Rules_From_Source_To_Ini(const std::string& source, INIClass& ini) const;

    RuleSection& Add_Section(std::string_view name, std::function<void(RuleSection&, std::string_view, const RuleValueVariant&)> on_rules_changed);

    RuleSection& Add_Section(std::string_view name);

    RuleSection& Get_Section(std::string_view name);

    void Clear();

    RuleSection& operator[](std::string_view name);

    const RuleSection& operator[](std::string_view name) const;

    JSON_FUNCTIONS(RuleSections)
private:
    static inline const auto& Logger = CncLogger::For(RuleSections);

    std::map<std::string, RuleSection> Sections;
    std::optional<std::function<void(RuleSection&, std::string_view, const RuleValueVariant&)>> OnRulesChangedDefault;
};

// std::format support for rule value variant
template <>
struct std::formatter<RuleValueVariant> : std::formatter<std::string> {
    auto format(RuleValueVariant value, format_context& ctx) const {
        return formatter<string>::format(RuleSection::Variant_To_String(value), ctx);
    }
};

// spdlog fmt library support for rule value variant (used by spdlog)
template <>
struct fmt::formatter<RuleValueVariant> : fmt::formatter<std::string> {
    auto format(const RuleValueVariant& value, fmt::format_context& ctx) const {
        return formatter<std::string>::format(RuleSection::Variant_To_String(value), ctx);
    }
};
