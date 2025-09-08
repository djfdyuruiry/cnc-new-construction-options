#pragma once

#include <concepts>

#include "../rulesections.h"

#include "luaapi.h"
#include "luaarguments.h"
#include "luatablebuilder.h"

/**
 * Describes a static class which has public static member
 * 'Sections' which provides a reference to a global
 * RuleSections instance.
 */
template <typename T>
concept RuleSectionsProviderConcept = requires() {
    { T::Sections } -> std::same_as<RuleSections&>;
};

/**
 * Provides a Lua interface for RuleSections.
 */
template <RuleSectionsProviderConcept R>
class RulesLuaApi : public LuaApi
{
public:
    RulesLuaApi() : LuaApi("Rules", true) {}

    virtual void Register_Functions(LuaEngine& engine) const override
    {
        With_Api_Namespace(engine, [](auto& n) {
            n.addCFunction("getSectionNames", [](auto L) {
                auto engine = SharedLuaEngine(L);

                auto section_names = R::Sections.Section_Names();
                auto table_builder = LuaTableBuilder(engine);

                for (const auto& name : section_names) {
                    table_builder.With_Index_Value(name);
                }

                return 1;
            }).addCFunction("getRuleNamesForSection", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Rules.getRuleNamesForSection(<string: section>)");

                arguments.Count_Is(1)
                    .First_Argument_Is<std::string>()
                    .Assert();

                auto section = arguments.Read_First<std::string>().Unpack();

                if (!R::Sections.Has_Section(section)) {
                    engine.Raise_Error(
                        std::format(
                            "Rule section does not exist: {}",
                            section
                        )
                    );
                }

                auto rule_names = R::Sections[section].Rule_Names();
                auto table_builder = LuaTableBuilder(engine);

                for (const auto& name : rule_names) {
                    table_builder.With_Index_Value(name);
                }

                return 1;
            }).addCFunction("getRuleType", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Rules.getRuleType(<string: section>, <string: key>)");

                arguments.Count_Is(2)
                    .First_Argument_Is<std::string>()
                    .Next_Argument_Is<std::string>()
                    .Assert();

                auto section = arguments.Read_First<std::string>().Unpack();
                auto key = arguments.Read_Next<std::string>().Unpack();

                Assert_Rule_Exists<R>(engine, section, key);

                // unpack variant to call corresponding engine Push_Value template
                const auto& rule_value_variant = R::Sections[section].Get_Variant(key);

                if (const auto* value = std::get_if<int>(&rule_value_variant)) {
                    engine.Push_Value(
                        LuaEngine::Lua_Type_Map[LUA_TNUMBER].value()
                    );
                } else if (const auto* value = std::get_if<float>(&rule_value_variant)) {
                    engine.Push_Value(
                        LuaEngine::Lua_Type_Map[LUA_TNUMBER].value()
                    );
                }else if (const auto* value = std::get_if<bool>(&rule_value_variant)) {
                    engine.Push_Value(
                        LuaEngine::Lua_Type_Map[LUA_TBOOLEAN].value()
                    );
                } else {
                    engine.Raise_Error("Illegal rule value variant detected");
                }

                return 1;
            }).addCFunction("getRuleValue", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Rules.getRuleValue(<string: section>, <string: key>)");

                arguments.Count_Is(2)
                    .First_Argument_Is<std::string>()
                    .Next_Argument_Is<std::string>()
                    .Assert();

                auto section = arguments.Read_First<std::string>().Unpack();
                auto key = arguments.Read_Next<std::string>().Unpack();

                Assert_Rule_Exists<R>(engine, section, key);

                // unpack variant to call corresponding engine Push_Value template
                const auto& rule_value_variant = R::Sections[section].Get_Variant(key);

                if (const auto* value = std::get_if<int>(&rule_value_variant)) {
                    engine.Push_Value(*value);
                } else if (const auto* value = std::get_if<float>(&rule_value_variant)) {
                    engine.Push_Value(*value);
                } else if (const auto* value = std::get_if<bool>(&rule_value_variant)) {
                    engine.Push_Value(*value);
                } else {
                    engine.Raise_Error("Illegal rule value variant detected");
                }

                return 1;
            }).addCFunction("setRuleValue", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Rules.setRuleValue(<string: section>, <string: key>, <number|int|bool: value>)");

                arguments.Count_Is(3)
                    .First_Argument_Is<std::string>()
                    .Next_Argument_Is<std::string>()
                    .Next_Argument_Is_Not_None()
                    .Assert();

                auto section = arguments.Read_First<std::string>().Unpack();
                auto key = arguments.Read_Next<std::string>().Unpack();

                Assert_Rule_Exists<R>(engine, section, key);

                // unpack variant to call corresponding section Set template
                const auto& rule_value_variant = R::Sections[section].Get_Variant(key);
                auto rule_type_error = true;
                auto expected_type = LUA_TNONE;

                if (const auto* value = std::get_if<int>(&rule_value_variant)) {
                    expected_type = LUA_TNUMBER;
    
                    if (arguments.template Next_Read_Is<int>()) {
                        R::Sections[section].template Set<int>(
                            key,
                            arguments.Read_Next<int>().Unpack()
                        );
                        rule_type_error = false;
                    } 
                } else if (const auto* value = std::get_if<float>(&rule_value_variant)) {
                    expected_type = LUA_TNUMBER;

                    if (arguments.template Next_Read_Is<float>()) {
                        R::Sections[section].template Set<float>(
                            key,
                            arguments.Read_Next<float>().Unpack()
                        );
                        rule_type_error = false;
                    }
                } else if (const auto* value = std::get_if<bool>(&rule_value_variant)) {
                    expected_type = LUA_TBOOLEAN;

                    if (arguments.template Next_Read_Is<bool>()) {
                        R::Sections[section].template Set<bool>(
                            key,
                            arguments.Read_Next<bool>().Unpack()
                        );
                        rule_type_error = false;
                    }
                } else {
                    engine.Raise_Error("Illegal rule value variant detected");
                }

                if (rule_type_error) {
                    engine.Raise_Error_Format(
                        "Incorrect type passed for rule value, expected '{}' but got: {}",
                        LuaEngine::Lua_Type_Map[expected_type].value(),
                        arguments.Get_Next_Read_Type()
                    );
                }

                // return old rule value
                if (const auto* value = std::get_if<int>(&rule_value_variant)) {
                    engine.Push_Value(*value);
                } else if (const auto* value = std::get_if<float>(&rule_value_variant)) {
                    engine.Push_Value(*value);
                } else if (const auto* value = std::get_if<bool>(&rule_value_variant)) {
                    engine.Push_Value(*value);
                }

                return 1;
            });
        });
    }

protected:
    template<RuleSectionsProviderConcept SR>
    static void Assert_Rule_Exists(const LuaEngine& engine, const std::string& section, const std::string& key) {
        if (!SR::Sections.Has_Section(section)) {
            engine.Raise_Error_Format(
                "Rule section does not exist: {}", 
                section
            );
        }

        if (!SR::Sections[section].Has_Key(key)) {
            engine.Raise_Error_Format(
                "Rule key does not exist in section '{}': {}", 
                section,
                key
            );
        }
    }

    virtual const char* Get_Cpp_Source() const override {
        return __FILE__;
    }

};
