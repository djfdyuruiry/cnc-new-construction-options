#pragma once

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

class RuleSection {
public:
    std::string_view SectionName;

    RuleSection(std::string_view section_name) : SectionName(section_name) {}

    template<typename T>
    RuleSection& With(INIClass& context, std::function<void(T&)> actions) {
        T contextWrapper = T(*this, context);

        actions(contextWrapper);

        return *this;
    }

    template<typename T>
    RuleSection& Load_From_Ini(INIClass& ini, std::string_view name, T default_value) {
        T value;

        auto sectionIsInIni = ini.Section_Present(SectionName.data());

        if (!sectionIsInIni) {
            CNC_LOG_INFO("Loading default for '{}', rule section not found in provided INI: [{}]", name, SectionName);

            Rules[name] = default_value;
            return *this;
        }

        CNC_LOG_DEBUG("Importing rule from INI: [{}] -> {}", SectionName, name);

        if constexpr (std::is_same_v<T, int>) {
            value = ini.Get_Int(SectionName.data(), name.data(), default_value);

            CNC_LOG_DEBUG("Resolved value: {} | (default={})", value, default_value);
        } else if constexpr (std::is_same_v<T, bool>) {
            value = ini.Get_Bool(SectionName.data(), name.data(), default_value);

            CNC_LOG_DEBUG("Resolved value: {} | (default={})", value, default_value);
        } else if constexpr (std::is_same_v<T, float>) {
            auto default_value_str = std::format("{}", default_value);
            value = std::stof(
                ini.Get_String(SectionName.data(), name.data(), default_value_str)
            );            

            CNC_LOG_DEBUG("Resolved value: {} | (default={})", value, default_value);
        } else {
            CNC_LOG_FATAL("Mapping for INI type not implemented, rule: [{}] -> {}", SectionName, name);
        }

        Rules[name] = value;

        return *this;
    }

    template<typename T>
    const RuleSection& Save_To_Ini(INIClass& ini, std::string_view name) const {
        auto value = Get<T>(name);

        CNC_LOG_DEBUG("Exporting rule to INI: [{}] -> {}", SectionName, name);

        if constexpr (std::is_same_v<T, int>) {
            ini.Put_Int(SectionName.data(), name.data(), value);

            CNC_LOG_DEBUG("Exported value: {}", value);
        } else if constexpr (std::is_same_v<T, bool>) {
            ini.Put_Bool(SectionName.data(), name.data(), value);

            CNC_LOG_DEBUG("Exported value: {}", value);
        } else if constexpr (std::is_same_v<T, float>) {
            auto value_str = std::format("{}", value);
            ini.Put_String(SectionName.data(), name.data(), value_str);

            CNC_LOG_DEBUG("Exported value: {}", value_str);
        } else {
            CNC_LOG_FATAL("Mapping for INI type not implemented, rule: [{}] -> {}", SectionName.data(), name.data());
        }

        return *this;
    }

    template<typename T>
    T Get(std::string_view name) const {
        auto it = Rules.find(name);

        if (it != Rules.end()) {
            return std::get<T>(it->second);
        }

        CNC_LOG_FATAL("Rule not found in section: [{}] -> {}", SectionName, name);
    }

    template<typename T>
    RuleSection& Set(std::string_view name, T value) {
        CNC_LOG_WARN("Updating rule at runtime: [{}] -> {}", SectionName, name);

        if constexpr (std::is_same_v<T, int> || std::is_same_v<T, bool> || std::is_same_v<T, float>) {
            CNC_LOG_WARN("New value: {}", value);
        } else {
            CNC_LOG_FATAL("Mapping for INI type not implemented, rule: [{}] -> {}", SectionName, name);
        }

        Rules[name] = value;

        return *this;
    }
private:
    using RuleVariant = std::variant<int, bool, float>;
    std::unordered_map<std::string_view, RuleVariant> Rules;
};

class IniRuleContext {
public:
    IniRuleContext(RuleSection& section, INIClass& context) : Section(section), Context(context) {}

    template<typename T>
    const IniRuleContext& Load(std::string_view name, T default_value) const {
        Section.Load_From_Ini(Context, name, default_value);

        return *this;
    }

    template<typename T>
    const IniRuleContext& Save(std::string_view name) const {
        Section.Save_To_Ini<T>(Context, name);

        return *this;
    }

    IniRuleContext& Load(std::string_view name) {
        NameInStream = std::make_optional(name);

        return *this;
    }

    template<typename T>
    IniRuleContext& With_Default(T default_value) {
        if (!NameInStream.has_value()) {
            CNC_LOG_FATAL("Load(..) must be called before With_Default(..)");
        }

        Load(NameInStream.value(), default_value);

        NameInStream = std::nullopt;

        return *this;
    }
private:
    RuleSection& Section;
    INIClass& Context;
    std::optional<std::string_view> NameInStream;
};

class RuleSections {
public:
    RuleSection& operator[](std::string_view name) {
        auto it = Sections.find(name);

        if (it != Sections.end()) {
            return *(it->second);
        }

        CNC_LOG_DEBUG("Adding new rules section '{}'", name.data());

        Sections[name] = std::make_unique<RuleSection>(name);

        return *Sections[name];
    }
private:
    std::unordered_map<std::string_view, std::unique_ptr<RuleSection>> Sections;
};
