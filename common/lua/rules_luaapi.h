#pragma once

#include <concepts>

#include "../rulesections.h"

#include "luaapi.h"

template <typename T>
concept RuleSectionsProviderConcept = requires() {
    { T::Sections } -> std::same_as<RuleSections&>;
};

template <RuleSectionsProviderConcept T> class RulesLuaApi : public LuaApi
{
public:
    RulesLuaApi(const LuaEngine& engine)
        : LuaApi(engine, "Rules", {"Rules.lua"})
    {
    }

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
            n.addCFunction("lookup", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Rules.get(<string: section>, <string: key>)");

                arguments.Count_Is(2)
                    .First_Argument_Is<std::string>()
                    .Next_Argument_Is<std::string>()
                    .Assert();

                auto section = arguments.Read_First<std::string>().Unpack();
                auto key = arguments.Read_Next<std::string>().Unpack();

                const RuleValueVariant& rule_value_variant = T::Sections[section].Get_Variant(key);

                if (const int* value = std::get_if<int>(&rule_value_variant)) {
                    engine.Push_Value(*value);
                } else if (const float* value = std::get_if<float>(&rule_value_variant)) {
                    engine.Push_Value(*value);
                } else if (const bool* value = std::get_if<bool>(&rule_value_variant)) {
                    engine.Push_Value(*value);
                } else {
                    engine.Raise_Error("Illegal rule value variant detected");
                }

                return 1;
            });
        });
    }
};
