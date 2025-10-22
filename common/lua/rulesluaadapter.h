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
        }
    }

    static void Push_Rule_Value(const LuaEngine& engine, RuleSections& sections, std::string section, std::string key)
    {
        Assert_Rule_Exists(engine, sections, section, key);

        // unpack variant to call corresponding engine Push_Value template
        const auto& rule_value_variant = sections[section].Get_Variant(key);

        if (const auto value = std::get_if<int>(&rule_value_variant)) {
            engine.Push_Value(*value);
        } else if (const auto value = std::get_if<uint>(&rule_value_variant)) {
            engine.Push_Value(*value);
        } else if (const auto value = std::get_if<char>(&rule_value_variant)) {
            engine.Push_Value(*value);
        } else if (const auto value = std::get_if<uchar>(&rule_value_variant)) {
            engine.Push_Value(*value);
        } else if (const auto value = std::get_if<ushort>(&rule_value_variant)) {
            engine.Push_Value(*value);
        } else if (const auto value = std::get_if<float>(&rule_value_variant)) {
            engine.Push_Value(*value);
        } else if (const auto value = std::get_if<bool>(&rule_value_variant)) {
            engine.Push_Value(*value);
        }
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

        if (const auto value = std::get_if<int>(&rule_value_variant)) {
            expected_type = LUA_TNUMBER;

            if (arguments.template Next_Read_Is<int>()) {
                sections[section].Set(
                    key,
                    arguments.Read_Next<int>().Unpack()
                );
                rule_type_error = false;
            }
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
        }

        if (rule_type_error) {
            engine.Raise_Error_Format(
                "Incorrect type passed for rule value, expected '{}' but got: {}",
                LuaEngine::LuaTypeMap[expected_type].value(),
                arguments.Get_Next_Read_Type()
            );
        }

        // return old rule value
        if (const auto value = std::get_if<int>(&rule_value_variant)) {
            engine.Push_Value(*value);
        } else if (const auto value = std::get_if<float>(&rule_value_variant)) {
            engine.Push_Value(*value);
        } else if (const auto value = std::get_if<bool>(&rule_value_variant)) {
            engine.Push_Value(*value);
        }
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
