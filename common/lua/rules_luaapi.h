#pragma once

#include <concepts>

#include "../rulesections.h"

#include "luaapi.h"

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
    RulesLuaApi(const LuaEngine& engine) 
     : LuaApi(engine, "Rules", {"Rules.lua"}) {}

    virtual void Register_Consts() const override
    {
        With_Api_Namespace([](auto& n) {
            //auto section_names = T::Sections.Section_Names();

            //n.addConstant("sectionNames", section_names);
        });
    }

    virtual void Register_Functions() const override
    {
        With_Api_Namespace([](auto& n) {
            n.addCFunction("get", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Rules.get(<string: section>, <string: key>)");

                arguments.Count_Is(2)
                    .First_Argument_Is<std::string>()
                    .Next_Argument_Is<std::string>()
                    .Assert();

                auto section = arguments.Read_First<std::string>().Unpack();
                auto key = arguments.Read_Next<std::string>().Unpack();

                if (!R::Sections.Has_Section(section)) {
                    engine.Raise_Error(
                        std::format(
                            "Rule section does not exist: {}",
                            section
                        )
                    );
                }

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
            }).addCFunction("set", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Rules.set(<string: section>, <string: key>, <number|int|bool: value>)");

                arguments.Count_Is(3)
                    .First_Argument_Is<std::string>()
                    .Next_Argument_Is<std::string>()
                    .Next_Argument_Is_Not_None()
                    .Assert();

                auto section = arguments.Read_First<std::string>().Unpack();
                auto key = arguments.Read_Next<std::string>().Unpack();

                if (!R::Sections.Has_Section(section)) {
                    engine.Raise_Error(
                        std::format(
                            "Rule section does not exist: {}", 
                            section
                        )
                    );
                }

                // unpack variant to call corresponding section Set template
                const auto& rule_value_variant = R::Sections[section].Get_Variant(key);
                auto rule_type_error = true;

                if (const auto* value = std::get_if<int>(&rule_value_variant)) {
                    if (arguments.template Next_Read_Is<int>()) {
                        R::Sections[section].template Set<int>(
                            key,
                            arguments.Read_Next<int>().Unpack()
                        );
                        rule_type_error = false;
                    } 
                } else if (const auto* value = std::get_if<float>(&rule_value_variant)) {
                    if (arguments.template Next_Read_Is<float>()) {
                        R::Sections[section].template Set<float>(
                            key,
                            arguments.Read_Next<float>().Unpack()
                        );
                        rule_type_error = false;
                    }
                } else if (const auto* value = std::get_if<bool>(&rule_value_variant)) {
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
                    engine.Raise_Error("Incorrect type passed for rule value");
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
};
