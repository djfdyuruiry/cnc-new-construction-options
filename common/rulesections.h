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
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <variant>

#include "fixed.h"
#include "ini.h"
#include "logger.h"

#include <map>

using RuleValueVariant = std::variant<int, bool, float, ushort, std::string>;

template<typename T>
concept RuleValueVariantCompatible = (
    std::is_same_v<T, int> ||
    std::is_same_v<T, bool> ||
    std::is_same_v<T, float> ||
    std::is_same_v<T, ushort> ||
    std::is_same_v<T, std::string>
);

template<typename C, typename T>
concept TypeConverter = requires(std::string str, T instance) {
    { C::template Try_Parse<T>(str) } -> std::same_as<std::optional<T>>;
    { C::template To_String<T>(instance) } -> std::same_as<std::string>;
};

class RuleSection
{
public:
    const std::string_view SectionName;

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
        }

        throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code"); 
    }

    RuleSection(
        std::string_view section_name,
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
        return Rules.find(name) != Rules.end();
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
        auto it = Rules.find(name);

        if (it != Rules.end()) {
            return it->second;
        }

        CNC_LOGGER_FATAL("Rule not found in section: [{}] -> {}", SectionName, name);
    }

    const std::optional<RuleValueVariant> Try_Get_Variant(std::string_view name) const
    {
        auto it = Rules.find(name);

        if (it != Rules.end()) {
            return it->second;
        }

        return std::nullopt;
    }

    const std::string_view Get_Type(std::string_view name) const
    {
        auto it = Rules.find(name);

        if (it != Rules.end()) {
            auto value_variant = it->second;

            return Get_Variant_Type(value_variant);
        }

        CNC_LOGGER_FATAL("Rule not found in section: [{}] -> {}", SectionName, name);
    }

    // TODO: Type Validation (for non trival types, unsigned/float etc.)/value error handling
    template<RuleValueVariantCompatible T>
    RuleSection& Load_From_Ini(INIClass& ini, std::string_view name, T default_value)
    {
        T value;

        auto sectionIsInIni = ini.Section_Present(SectionName.data());

        if (!sectionIsInIni) {
            CNC_LOGGER_INFO(
                "Loading default value '{}' for '{}', rule section not found in provided INI: [{}]",
                Variant_To_String(default_value),
                name,
                SectionName
            );

            Rules[name] = default_value;
            return *this;
        }

        CNC_LOGGER_DEBUG(
            "Attempting to find rule in INI: [{}] -> {} (with default = {})",
            SectionName,
            name,
            Variant_To_String(default_value)
        );

        if constexpr (std::is_same_v<T, int>) {
            value = ini.Get_Int(SectionName.data(), name.data(), default_value);
        } else if constexpr (std::is_same_v<T, bool>) {
            value = ini.Get_Bool(SectionName.data(), name.data(), default_value);
        } else if constexpr (std::is_same_v<T, float>) {
            auto default_value_str = std::format("{}", default_value);

            value = std::stof(
                ini.Get_String(SectionName.data(), name.data(), default_value_str)
            );
        } else if constexpr (std::is_same_v<T, ushort>) {
            auto ini_value = ini.Get_Int(SectionName.data(), name.data(), default_value);

            if (ini_value < 0 || ini_value > std::numeric_limits<ushort>::max()) {
                CNC_LOGGER_FATAL(
                    "Invalid INI value - number '{}' is out of expected range: {} - {}",
                    ini_value,
                    0,
                    std::numeric_limits<ushort>::max()
                );
            }

            value = static_cast<ushort>(ini_value);
        } else if constexpr (std::is_same_v<T, std::string>) {
            auto str_value = ini.Get_String(SectionName.data(), name.data(), default_value);

            // TODO: trim string to forgive spacing around rule string
            // forgive incorrect casing in rule values
            std::transform(str_value.begin(), str_value.end(), str_value.begin(), ::toupper);

            value = str_value;
        }

        CNC_LOGGER_DEBUG(
            "Imported rule from INI: [{}] -> {} = {}",
            SectionName,
            name,
            Variant_To_String(value)
        );

        Rules[name] = value;

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
            auto value_str = std::format("{}", *value);
            ini.Put_String(SectionName.data(), name.data(), value_str);
        } else if (const auto value = std::get_if<ushort>(&value_variant)) {
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
    std::optional<T> Get_With_Converter(std::string_view name) const
    {
        return C::template Try_Parse<T>(
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

        Rules[name] = value;

        CNC_LOGGER_WARN("Running OnRulesChanged() handler");
        OnRulesChanged();

        return *this;
    }

    template<class T, TypeConverter<T> C>
    RuleSection& Set_With_Converter(std::string_view name, T instance) const
    {
        return Set(name, C::To_String(instance));
    }

private:
    inline static CncLogger Logger = CncLogger("RuleSection");

    std::map<std::string_view, RuleValueVariant> Rules;
    std::function<void(void)> OnRulesChanged;
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

    template<RuleValueVariantCompatible T>
    const IniRuleContext& Load_With_Callback(std::string_view name, T default_value, std::function<void(T)> callback) const
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
        Load<std::string>(name, C::template To_String<T>(default_value));

        callback(Section.Get_With_Converter<T, C>(name).value_or(default_value));

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
    inline static CncLogger Logger = CncLogger("IniRuleContext");

    RuleSection& Section;
    INIClass& Context;
    std::optional<std::string_view> NameInStream;
};

// IniRuleContext macro 'methods' - useful for setting variables and class members from rules

// Load a variable/member by its C++ name (with a specific type) from an INI context and set its value
// to equal the INI value
#define Load_Var_With_Type(VAR, T) Load_With_Callback<T>(#VAR, VAR, [&](const auto v) { VAR = v; })

// Load a variable/member by its C++ name from an INI context and set its value to equal the INI value
#define Load_Var(VAR) Load_With_Callback(#VAR, VAR, [&](const auto v) { VAR = v; })
#define Load_Bool_Var(VAR) Load_Var_With_Type(VAR, bool)
#define Load_UShort_Var(VAR) Load_Var_With_Type(VAR, ushort)
#define Load_Int_Var(VAR) Load_Var_With_Type(VAR, int)

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
        return Sections.find(name) != Sections.end();
    }

    void Save_All_To_Ini(INIClass& ini) const
    {
        for (const auto& section : Sections | std::views::values) {
            section->Save_All_To_Ini(ini);
        }
    }

    RuleSection& operator[](std::string_view name)
    {
        auto it = Sections.find(name);

        if (it != Sections.end()) {
            return *(it->second);
        }

        CNC_LOGGER_DEBUG("Adding new rules section '{}'", name);

        Sections[name] = std::make_unique<RuleSection>(
            name,
            [&]() {
                if (OnRulesChanged.has_value()) {
                    OnRulesChanged.value()();
                }
            }
        );

        return *Sections[name];
    }

private:
    inline static CncLogger Logger = CncLogger("RuleSections");

    std::map<std::string_view, std::unique_ptr<RuleSection>> Sections;
    std::optional<std::function<void(void)>> OnRulesChanged;
};
