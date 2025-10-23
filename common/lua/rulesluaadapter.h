#pragma once

#include <lua.h>

#include "luaengine.h"
#include "rulesections.h"

class LuaEngine;class RulesLuaAdapter final
{
public:
    static void Push_Rule_Type(const LuaEngine& engine, RuleSections& sections, std::string section, std::string key)
    {
        Assert_Rule_Exists(engine, sections, section, key);

        const auto& rule_value_variant = sections[section].Get_Variant(key);

        if (
            std::get_if<int>(&rule_value_variant) ||
            std::get_if<uint>(&rule_value_variant) ||
            std::get_if<float>(&rule_value_variant) ||
            std::get_if<ushort>(&rule_value_variant) ||
            std::get_if<char>(&rule_value_variant) ||
            std::get_if<uchar>(&rule_value_variant)
        ) {
            engine.Push_Value(
                LuaEngine::LuaTypeMap[LUA_TNUMBER].value()
            );
        } else if (std::get_if<bool>(&rule_value_variant)) {
            engine.Push_Value(
                LuaEngine::LuaTypeMap[LUA_TBOOLEAN].value()
            );
        } else if (std::get_if<std::string>(&rule_value_variant)) {
            engine.Push_Value(
                LuaEngine::LuaTypeMap[LUA_TSTRING].value()
            );
        } else {
            throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
        }
    }

    /**
     * Unpack variant to call corresponding engine Push_Value template.
     */
    static void Push_Rule_Variant(const LuaEngine& engine, const RuleValueVariant& variant)
    {
        if (const auto value = std::get_if<int>(&variant)) {
            engine.Push_Value(*value);
        } else if (const auto value = std::get_if<uint>(&variant)) {
            engine.Push_Value(*value);
        } else if (const auto value = std::get_if<char>(&variant)) {
            engine.Push_Value(*value);
        } else if (const auto value = std::get_if<uchar>(&variant)) {
            engine.Push_Value(*value);
        } else if (const auto value = std::get_if<ushort>(&variant)) {
            engine.Push_Value(*value);
        } else if (const auto value = std::get_if<float>(&variant)) {
            engine.Push_Value(*value);
        } else if (const auto value = std::get_if<bool>(&variant)) {
            engine.Push_Value(*value);
        } else if (const auto value = std::get_if<std::string>(&variant)) {
            engine.Push_Value(*value);
        } else {
            throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
        }
    }

    static void Push_Rule_Value(const LuaEngine& engine, RuleSections& sections, std::string section, std::string key)
    {
        Assert_Rule_Exists(engine, sections, section, key);
        Push_Rule_Variant(engine, sections[section].Get_Variant(key));
    }

    template<LuaVariantCompatible T, RuleValueVariantCompatible U>
    static bool Set_Rule_Value_For_Section(LuaArguments& args, RuleSection& section, const std::string& key)
    {
        if (!args.template Next_Read_Is<T>()) {
            return false;
        }

        section.Set(
            key,
            static_cast<U>(args.Read_Next<int>().Unpack())
        );

        return true;
    }

    /**
     * Pull a Lua value from the current parameters and use it to set the value of a given
     * rule, with type checking. The old rule value is pushed onto the Lua stack if set was
     * successful.
     *
     * @param engine Context of Lua call
     * @param arguments Lua arguments passed, next argument in chain must be the rule value
     * @param sections Rules root
     * @param section Context of rule set
     * @param key Rule key to set
     */
    static void Set_Rule_Value(
        const SharedLuaEngine& engine,
        LuaArguments& arguments,
        RuleSections& sections,
        const std::string& section,
        const std::string& key
    )
    {
        Assert_Rule_Exists(engine, sections, section, key);

        // unpack variant to call corresponding section Set template
        const auto& rule_value_variant = sections[section].Get_Variant(key);
        auto rule_type_error = true;
        auto expected_type = LUA_TNONE;

        if (
            std::get_if<int>(&rule_value_variant) ||
            std::get_if<uint>(&rule_value_variant) ||
            std::get_if<ushort>(&rule_value_variant) ||
            std::get_if<char>(&rule_value_variant) ||
            std::get_if<uchar>(&rule_value_variant)
        ) {
            expected_type = LUA_TNUMBER;
        }

        if (std::get_if<int>(&rule_value_variant)) {
            rule_type_error = !Set_Rule_Value_For_Section<int, int>(arguments, sections[section], key);
        } else if (std::get_if<uint>(&rule_value_variant)) {
            rule_type_error = !Set_Rule_Value_For_Section<int, uint>(arguments, sections[section], key);
        } else if (std::get_if<ushort>(&rule_value_variant)) {
            rule_type_error = !Set_Rule_Value_For_Section<int, ushort>(arguments, sections[section], key);
        } else if (std::get_if<char>(&rule_value_variant)) {
            rule_type_error = !Set_Rule_Value_For_Section<int, char>(arguments, sections[section], key);
        } else if (std::get_if<uchar>(&rule_value_variant)) {
            rule_type_error = !Set_Rule_Value_For_Section<int, uchar>(arguments, sections[section], key);
        } else if (const auto value = std::get_if<float>(&rule_value_variant)) {
            expected_type = LUA_TNUMBER;

            if (arguments.template Next_Read_Is<float>()) {
                sections[section].Set(
                    key,
                    arguments.Read_Next<float>().Unpack()
                );
                rule_type_error = false;
            }
        } else if (const auto value = std::get_if<bool>(&rule_value_variant)) {
            expected_type = LUA_TBOOLEAN;

            if (arguments.template Next_Read_Is<bool>()) {
                sections[section].Set(
                    key,
                    arguments.Read_Next<bool>().Unpack()
                );
                rule_type_error = false;
            }
        } else if (const auto value = std::get_if<std::string>(&rule_value_variant)) {
            expected_type = LUA_TSTRING;

            if (arguments.template Next_Read_Is<std::string>()) {
                sections[section].Set(
                    key,
                    arguments.Read_Next<std::string>().Unpack()
                );
                rule_type_error = false;
            }
        } else {
            throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
        }

        if (rule_type_error) {
            engine.Raise_Error_Format(
                "Incorrect type passed for rule value, expected '{}' but got: {}",
                LuaEngine::LuaTypeMap[expected_type].value(),
                arguments.Get_Next_Read_Type()
            );
        }

        // return old rule value to lua caller
        Push_Rule_Variant(engine, rule_value_variant);
    }

    static void Assert_Rule_Exists(const LuaEngine& engine, RuleSections& sections, std::string section, std::string key)
    {
        if (!sections.Has_Section(section)) {
            engine.Raise_Error_Format(
                "Rule section does not exist: {}",
                section
            );
        }

        if (!sections[section].Has_Key(key)) {
            engine.Raise_Error_Format(
                "Rule key does not exist in section '{}': {}",
                section,
                key
            );
        }
    }

private:
    RulesLuaAdapter() = delete;
};
