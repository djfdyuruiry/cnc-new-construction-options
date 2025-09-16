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
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <variant>

#include "fixed.h"
#include "ini.h"
#include "logger.h"

using RuleValueVariant = std::variant<int, bool, float>;

template<typename T>
concept RuleValueVariantCompatible = (
    std::is_same_v<T, int> || std::is_same_v<T, bool> || std::is_same_v<T, float>
);

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
        }
    }

    static std::string_view Get_Variant_Type(RuleValueVariant value_variant)
    {
        if (const auto value = std::get_if<int>(&value_variant)) {
            return "int";
        } else if (const auto value = std::get_if<bool>(&value_variant)) {
            return "bool";
        } else if (const auto value = std::get_if<float>(&value_variant)) {
            return "float";
        }
    }

    static std::string Variant_To_String(RuleValueVariant value_variant)
    {
        if (const auto value = std::get_if<int>(&value_variant)) {
            return std::format("{}", *value);
        } else if (const auto value = std::get_if<bool>(&value_variant)) {
            return std::format("{}", *value);
        } else if (const auto value = std::get_if<float>(&value_variant)) {
            return std::format("{}", *value);
        }
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

        for (const auto& pair : Rules) {
            keys.emplace_back(pair.first);
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

    // TODO: Validation/value error handling
    template<RuleValueVariantCompatible T>
    RuleSection& Load_From_Ini(INIClass& ini, std::string_view name, T default_value)
    {
        T value;

        auto sectionIsInIni = ini.Section_Present(SectionName.data());

        if (!sectionIsInIni) {
            CNC_LOGGER_INFO("Loading default for '{}', rule section not found in provided INI: [{}]", name, SectionName);

            Rules[name] = default_value;
            return *this;
        }

        CNC_LOGGER_DEBUG("Importing rule from INI: [{}] -> {}", SectionName, name);

        if constexpr (std::is_same_v<T, int>) {
            value = ini.Get_Int(SectionName.data(), name.data(), default_value);

            CNC_LOGGER_DEBUG("Resolved value: {} | (default={})", value, default_value);
        } else if constexpr (std::is_same_v<T, bool>) {
            value = ini.Get_Bool(SectionName.data(), name.data(), default_value);

            CNC_LOGGER_DEBUG("Resolved value: {} | (default={})", value, default_value);
        } else if constexpr (std::is_same_v<T, float>) {
            auto default_value_str = std::format("{}", default_value);
            value = std::stof(
                ini.Get_String(SectionName.data(), name.data(), default_value_str)
            );

            CNC_LOGGER_DEBUG("Resolved value: {} | (default={})", value, default_value);
        }

        Rules[name] = value;

        return *this;
    }

    const RuleSection& Save_To_Ini(INIClass& ini, std::string_view name) const
    {
        CNC_LOGGER_DEBUG("Exporting rule to INI: [{}] -> {}", SectionName, name);

        auto value_variant = Get_Variant(name);

        if (const auto value = std::get_if<int>(&value_variant)) {
            ini.Put_Int(SectionName.data(), name.data(), *value);

            CNC_LOGGER_DEBUG("Exported value: {}", *value);
        } else if (const auto value = std::get_if<bool>(&value_variant)) {
            ini.Put_Bool(SectionName.data(), name.data(), *value);

            CNC_LOGGER_DEBUG("Exported value: {}", *value);
        } else if (const auto value = std::get_if<float>(&value_variant)) {
            auto value_str = std::format("{}", *value);
            ini.Put_String(SectionName.data(), name.data(), value_str);

            CNC_LOGGER_DEBUG("Exported value: {}", value_str);
        }

        return *this;
    }

    template<RuleValueVariantCompatible T>
    std::optional<T> Try_Get(std::string_view name) const
    {
        auto value_variant_optional = Try_Get_Variant(name);

        if (!value_variant_optional.has_value()) {
            return std::nullopt;
        }

        auto value_variant = value_variant_optional.value();

        if constexpr (std::is_same_v<T, int>) {
            if (const auto value = std::get_if<int>(&value_variant)) {
                return *value;
            }
        }
        
        if constexpr (std::is_same_v<T, bool>) {
            if (const auto value = std::get_if<bool>(&value_variant)) {
                return *value;
            }
        }

        if constexpr (std::is_same_v<T, float>) {
            if (const auto value = std::get_if<float>(&value_variant)) {
                return *value;
            }
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

private:
    inline static CncLogger Logger = CncLogger("RuleSection");

    std::unordered_map<std::string_view, RuleValueVariant> Rules;
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

    std::unordered_map<std::string_view, std::unique_ptr<RuleSection>> Sections;
    std::optional<std::function<void(void)>> OnRulesChanged;
};
