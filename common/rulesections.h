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
#include <ranges>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <variant>

#include "fixed.h"
#include "ini.h"
#include "logger.h"

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned char uchar;

using RuleValueVariant = std::variant<int, bool, float, ushort, std::string, uint, char, uchar>;

template<typename T>
concept RuleValueVariantCompatible = (
    std::is_same_v<T, int> ||
    std::is_same_v<T, bool> ||
    std::is_same_v<T, float> ||
    std::is_same_v<T, ushort> ||
    std::is_same_v<T, std::string> ||
    std::is_same_v<T, uint> ||
    std::is_same_v<T, char>||
    std::is_same_v<T, uchar>
);

template<typename C, typename T>
concept TypeConverter = requires(std::string str, const std::string& str_ref, const std::vector<T>& instances, const char c, T instance) {
    { C::template Try_Parse<T>(str) } -> std::same_as<std::optional<T>>;
    { C::template Try_Parse_Csv<T>(str_ref, c) } -> std::same_as<std::vector<T>>;
    { C::template To_String<T>(instance) } -> std::same_as<std::string>;
    { C::template To_Csv_String<T>(instances) }  -> std::same_as<std::string>;
};

class RuleSection
{
public:
    const std::string SectionName;

    static bool Variants_Have_Same_Type(RuleValueVariant value_variant_a, RuleValueVariant value_variant_b)
    {
        if (const auto value = std::get_if<int>(&value_variant_a)) {
            return std::get_if<int>(&value_variant_b) != nullptr;
        } else if (const auto value = std::get_if<bool>(&value_variant_a)) {
            return std::get_if<bool>(&value_variant_b) != nullptr;
        } else if (const auto value = std::get_if<float>(&value_variant_a)) {
            return std::get_if<float>(&value_variant_b) != nullptr;
        } else if (const auto value = std::get_if<ushort>(&value_variant_a)) {
            return std::get_if<ushort>(&value_variant_b) != nullptr;
        } else if (const auto value = std::get_if<std::string>(&value_variant_a)) {
            return std::get_if<std::string>(&value_variant_b) != nullptr;
        } else if (const auto value = std::get_if<uint>(&value_variant_a)) {
            return std::get_if<uint>(&value_variant_b) != nullptr;
        } else if (const auto value = std::get_if<char>(&value_variant_a)) {
            return std::get_if<char>(&value_variant_b) != nullptr;
        } else if (const auto value = std::get_if<uchar>(&value_variant_a)) {
            return std::get_if<uchar>(&value_variant_b) != nullptr;
        }

        throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
    }

    static std::string_view Get_Variant_Type(RuleValueVariant value_variant)
    {
        if (const auto value = std::get_if<int>(&value_variant)) {
            return "int";
        } else if (const auto value = std::get_if<bool>(&value_variant)) {
            return "bool";
        } else if (const auto value = std::get_if<float>(&value_variant)) {
            return "float";
        } else if (const auto value = std::get_if<ushort>(&value_variant)) {
            return "ushort";
        } else if (const auto value = std::get_if<std::string>(&value_variant)) {
            return "string";
        } else if (const auto value = std::get_if<uint>(&value_variant)) {
            return "uint";
        } else if (const auto value = std::get_if<char>(&value_variant)) {
            return "char";
        } else if (const auto value = std::get_if<uchar>(&value_variant)) {
            return "uchar";
        }

        throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
    }

    static std::string Variant_To_String(RuleValueVariant value_variant)
    {
        if (const auto value = std::get_if<int>(&value_variant)) {
            return std::format("{}", *value);
        } else if (const auto value = std::get_if<bool>(&value_variant)) {
            return std::format("{}", *value);
        } else if (const auto value = std::get_if<float>(&value_variant)) {
            return std::format("{}", *value);
        } else if (const auto value = std::get_if<ushort>(&value_variant)) {
            return std::format("{}", *value);
        } else if (const auto value = std::get_if<std::string>(&value_variant)) {
            return *value;
        } else if (const auto value = std::get_if<uint>(&value_variant)) {
            return std::format("{}", *value);
        } else if (const auto value = std::get_if<char>(&value_variant)) {
            return std::format("{}", static_cast<int>(*value));
        } else if (const auto value = std::get_if<uchar>(&value_variant)) {
            return std::format("{}", static_cast<unsigned int>(*value));
        }

        throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
    }

    RuleSection(
        std::string section_name,
        std::function<void(void)> on_rules_changed
    ) : SectionName(section_name), OnRulesChanged(on_rules_changed) {}

    template<typename T>
    RuleSection& With(INIClass& context, std::function<void(T&)> actions)
    {
        T contextWrapper = T(*this, context);

        actions(contextWrapper);

        return *this;
    }

    bool Has_Key(std::string_view name)
    {
        return Rules.find(std::string(name)) != Rules.end();
    }

    std::vector<std::string_view> Rule_Names() const
    {
        std::vector<std::string_view> keys;
        keys.reserve(Rules.size());

        for (const auto& key : Rules | std::views::keys) {
            keys.emplace_back(key);
        }

        return keys;
    }

    const RuleValueVariant Get_Variant(std::string_view name) const
    {
        auto it = Rules.find(std::string(name));

        if (it != Rules.end()) {
            return it->second;
        }

        CNC_LOGGER_FATAL("Rule not found in section: [{}] -> {}", SectionName, name);
    }

    const std::optional<RuleValueVariant> Try_Get_Variant(std::string_view name) const
    {
        auto it = Rules.find(std::string(name));

        if (it != Rules.end()) {
            return it->second;
        }

        return std::nullopt;
    }

    const std::string_view Get_Type(std::string_view name) const
    {
        auto it = Rules.find(std::string(name));

        if (it != Rules.end()) {
            auto value_variant = it->second;

            return Get_Variant_Type(value_variant);
        }

        CNC_LOGGER_FATAL("Rule not found in section: [{}] -> {}", SectionName, name);
    }

    // TODO: Type Validation (for non trivial types, unsigned/float etc.)/value error handling
    template<RuleValueVariantCompatible T>
    RuleSection& Load_From_Ini(
        INIClass& ini,
        std::string_view name,
        T default_value,
        const std::optional<std::function<bool(RuleValueVariant)>>& str_validator = std::nullopt
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

            Rules[std::string(name)] = default_value;
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
                ini.Get_String(SectionName.data(), name.data(), default_value_str),
                value,
                std::stof
            );
        } else if constexpr (std::is_same_v<T, ushort>) {
            auto default_value_str = std::format("{}", default_value);

            Safe_Parse<ushort, ulong>(
                ini.Get_String(SectionName.data(), name.data(), default_value_str),
                value,
                std::stoul,
                [](auto v) {
                    return v >= std::numeric_limits<ushort>::min() || v <= std::numeric_limits<ushort>::max();
                }
            );
        } else if constexpr (std::is_same_v<T, uint>) {
            auto default_value_str = std::format("{}", default_value);

            Safe_Parse<uint, ulong>(
                ini.Get_String(SectionName.data(), name.data(), default_value_str),
                value,
                std::stoul,
                [](auto v) {
                    return v >= std::numeric_limits<uint>::min() || v <= std::numeric_limits<uint>::max();
                }
            );
        } else if constexpr (std::is_same_v<T, char>) {
            Safe_Parse_Int<char>(
                ini.Get_Int(SectionName.data(), name.data(), default_value),
                value,
                [](auto v) {
                    return v >= std::numeric_limits<char>::min() || v <= std::numeric_limits<char>::max();
                }
            );
        } else if constexpr (std::is_same_v<T, uchar>) {
            Safe_Parse_Int<uchar>(
                ini.Get_Int(SectionName.data(), name.data(), default_value),
                value,
                [](auto v) {
                    return v >= std::numeric_limits<uchar>::min() || v <= std::numeric_limits<uchar>::max();
                }
            );
        } else if constexpr (std::is_same_v<T, std::string>) {
            auto str_value = ini.Get_String(SectionName.data(), name.data(), default_value);

            // TODO: trim string to forgive spacing around rule string
            // forgive incorrect casing in rule values
            std::transform(str_value.begin(), str_value.end(), str_value.begin(), ::toupper);

            if (str_validator.has_value() && ! str_validator.value()(value)) {
                CNC_LOGGER_ERROR(
                    "Invalid INI value '{}' for rule: [{}] -> {}",
                    str_value,
                    SectionName,
                    name
                );

                return *this;
            }

            value = str_value;
        }

        CNC_LOGGER_DEBUG(
            "Imported rule from INI: [{}] -> {} = {}",
            SectionName,
            name,
            Variant_To_String(value)
        );

        Rules[std::string(name)] = value;

        return *this;
    }

    const RuleSection& Save_To_Ini(INIClass& ini, std::string_view name) const
    {
        auto value_variant = Get_Variant(name);

        CNC_LOGGER_DEBUG(
            "Exporting rule to INI: [{}] -> {} = {}",
            SectionName,
            name,
            Variant_To_String(value_variant)
        );

        if (const auto value = std::get_if<int>(&value_variant)) {
            ini.Put_Int(SectionName.data(), name.data(), *value);
        } else if (const auto value = std::get_if<bool>(&value_variant)) {
            ini.Put_Bool(SectionName.data(), name.data(), *value);
        } else if (const auto value = std::get_if<float>(&value_variant)) {
            const auto value_str = std::format("{}", *value);
            ini.Put_String(SectionName.data(), name.data(), value_str);
        } else if (const auto value = std::get_if<ushort>(&value_variant)) {
            const auto value_str = std::format("{}", *value);
            ini.Put_String(SectionName.data(), name.data(), value_str);
        } else if (const auto value = std::get_if<uint>(&value_variant)) {
            const auto value_str = std::format("{}", *value);
            ini.Put_String(SectionName.data(), name.data(), value_str);
        } else if (const auto value = std::get_if<char>(&value_variant)) {
            ini.Put_Int(SectionName.data(), name.data(), *value);
        } else if (const auto value = std::get_if<uchar>(&value_variant)) {
            ini.Put_Int(SectionName.data(), name.data(), *value);
        } else if (const auto value = std::get_if<std::string>(&value_variant)) {
            ini.Put_String(SectionName.data(), name.data(), *value);
        }

        return *this;
    }

    void Save_All_To_Ini(INIClass& ini) const
    {
        for (const auto& key : Rules | std::views::keys) {
            Save_To_Ini(ini, key);
        }
    }

    template<RuleValueVariantCompatible T>
    [[nodiscard]]
    std::optional<T> Try_Get(std::string_view name) const
    {
        auto value_variant_optional = Try_Get_Variant(name);

        if (!value_variant_optional.has_value()) {
            return std::nullopt;
        }

        auto value_variant = value_variant_optional.value();

        if (const auto value = std::get_if<T>(&value_variant)) {
            return *value;
        }

        CNC_LOGGER_FATAL(
            "Attempted to read rule using wrong type '{}' (correct type: {}), found in section: [{}] -> {}",
            typeid(T).name(),
            Get_Type(name),
            SectionName,
            name
        );
    }

    template<RuleValueVariantCompatible T>
    T Get(std::string_view name) const
    {
        auto value_optional = Try_Get<T>(name);

        if (!value_optional.has_value()) {
            CNC_LOGGER_FATAL("Rule not found in section: [{}] -> {}", SectionName, name);
        }

        return value_optional.value();
    }

    template<class T, TypeConverter<T> C>
    T Get_With_Converter(std::string_view name) const
    {
        auto str_value = Get<std::string>(name);

        // we validate all INI rules and refuse to set invalid rules at runtime, so stored
        // value is always parsable
        C::template Try_Parse<T>(str_value).value();
    }

    template<class T, TypeConverter<T> C>
    std::vector<T> Get_With_Csv_Converter(std::string_view name) const
    {
        return C::template Try_Parse_Csv<T>(
            Get<std::string>(name)
        );
    }

    RuleSection& Set(std::string_view name, RuleValueVariant value)
    {
        CNC_LOGGER_WARN(
            "Updating rule at runtime: [{}] -> {} = {}",
            SectionName,
            name,
            Variant_To_String(value)
        );

        auto existing_rule = Try_Get_Variant(name);

        if (existing_rule.has_value()) {
            if (!Variants_Have_Same_Type(existing_rule.value(), value)) {
                CNC_LOGGER_FATAL(
                    "Attempted to set rule using wrong type '{}' (correct type: {}), found in section: [{}] -> {}",
                    Get_Variant_Type(value),
                    Get_Variant_Type(existing_rule.value()),
                    SectionName,
                    name
                );
            }
        }

        Rules[std::string(name)] = value;

        CNC_LOGGER_WARN("Running OnRulesChanged() handler");
        OnRulesChanged();

        return *this;
    }

    template<class T, TypeConverter<T> C>
    RuleSection& Set_With_Converter(std::string_view name, T instance) const
    {
        Set(name, C::To_String(instance));
    }

    template<class T, TypeConverter<T> C>
    RuleSection& Set_With_Converter(std::string_view name, std::string instance_string) const
    {
        auto parsed_instance = C::template Try_Parse<T>(instance_string);

        return Set(name, C::To_String(parsed_instance.value()));
    }

    template<class T, TypeConverter<T> C>
    RuleSection& Set_With_Csv_Converter(std::string_view name, const std::vector<T>& instances) const
    {
        return Set(name, C::To_Csv_String(instances));
    }

    template<class T, TypeConverter<T> C>
    RuleSection& Set_With_Csv_Converter(std::string_view name, std::string instances_csv) const
    {
        auto parsed_instance = C::template Try_Parse_Csv<T>(instances_csv);

        return Set(name, C::To_Csv_String(parsed_instance));
    }
private:
    static inline const auto& Logger = CncLogger::For(RuleSection);

    std::map<std::string, RuleValueVariant> Rules;
    std::function<void(void)> OnRulesChanged;

    template<class T, class U = T, class V = std::string>
    void Safe_Parse(
        V source,
        T& target,
        std::function<U(const V&)> parse,
        std::function<bool(U)> validate = [](auto _) { return true; }
    )
    {
        std::optional<std::string> ex_type;
        std::optional<T> parsed_value;

        try {
            parsed_value = parse(source);

            if (validate(parsed_value)) {
                target = static_cast<T>(parsed_value);
                return;
            }
        } catch (...) {
            ex_type = std::current_exception().__cxa_exception_type()->name();
        }

        CNC_LOGGER_ERROR(
            std::vformat(
                "Invalid INI value '{}' for rule: [{}] -> {}{}{}",
                std::make_format_args(
                    source,
                    parsed_value.has_value() ? std::format(" parsed_value={}", parsed_value.value()) : "",
                    ex_type.has_value() ? std::format(" parsed_error={}", ex_type.value()) : ""
                )
           )
        );
    }

    template<class T>
    void Safe_Parse_Int(
        int source,
        T& target,
        std::function<bool(T)> validate
    )
    {
        Safe_Parse<T, T, int>(
            source,
            target,
            [](const auto& v) { return static_cast<T>(v); },
            validate
        );
    }
};

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
        Section.Load_From_Ini<std::string>(Context, name, C::To_String(default_value));

        return *this;
    }

    template<class T, TypeConverter<T> C>
    const IniRuleContext& Load_With_Csv_Converter(std::string_view name, const std::vector<T>& default_values) const
    {
        Section.Load_From_Ini<std::string>(Context, name, C::To_Csv_String(default_values));

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

    const IniRuleContext& Save(std::string_view name) const
    {
        Section.Save_To_Ini(Context, name);

        return *this;
    }

    IniRuleContext& Load(std::string_view name)
    {
        NameInStream = std::make_optional(name);

        return *this;
    }

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
    void On_Rules_Changed(std::function<void(void)> on_rules_changed)
    {
        OnRulesChanged = on_rules_changed;
    }

    std::vector<std::string_view> Section_Names() const
    {
        std::vector<std::string_view> keys;
        keys.reserve(Sections.size());

        for (const auto& pair : Sections) {
            keys.emplace_back(pair.first);
        }

        return keys;
    }

    bool Has_Section(std::string_view name)
    {
        return Sections.find(std::string(name)) != Sections.end();
    }

    void Save_All_To_Ini(INIClass& ini) const
    {
        for (const auto& section : Sections | std::views::values) {
            section->Save_All_To_Ini(ini);
        }
    }

    RuleSection& operator[](std::string_view name)
    {
        auto name_str = std::string(name);
        auto it = Sections.find(name_str);

        if (it != Sections.end()) {
            return *(it->second);
        }

        CNC_LOGGER_DEBUG("Adding new rules section '{}'", name);

        Sections[name_str] = std::make_unique<RuleSection>(
            name_str,
            [&]() {
                if (OnRulesChanged.has_value()) {
                    OnRulesChanged.value()();
                }
            }
        );

        return *Sections[name_str];
    }

private:
    static inline const auto& Logger = CncLogger::For(RuleSections);

    std::map<std::string, std::unique_ptr<RuleSection>> Sections;
    std::optional<std::function<void()>> OnRulesChanged;
};
