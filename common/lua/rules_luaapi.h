#pragma once

#include <concepts>

#include "rulesections.h"

#include "luaapi.h"
#include "luaarguments.h"
#include "luatablebuilder.h"
#include "rulesluaadapter.h"

/**
 * Describes a static class which has public static member
 * 'Sections' which provides a reference to a global
 * RuleSections instance.
 */
template <typename T>
concept RuleSectionsProviderConcept = requires()
{
    { T::Sections() } -> std::same_as<const RuleSections&>;
    { T::Editable_Sections() } -> std::same_as<RuleSections&>;
};

// TODO: Update to support new rule variant types
/**
 * Provides a Lua interface for RuleSections.
 */
template <RuleSectionsProviderConcept R>
class RulesLuaApi : public LuaApi
{
public:
    RulesLuaApi() : LuaApi("Rules", true) {}

    void Register_Functions(LuaEngine& engine) const override
    {
        With_Api_Namespace(engine, [](auto& n) {
            n.addCFunction("getSectionNames", [](auto L) {
                auto engine = SharedLuaEngine(L);

                auto section_names = R::Sections().Section_Names();
                auto table_builder = LuaTableBuilder::Push_New_Table(engine);

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

                if (!R::Sections().Has_Section(section)) {
                    engine.Raise_Error(
                        std::format(
                            "Rule section does not exist: {}",
                            section
                        )
                    );
                }

                auto rule_names = R::Sections()[section].Rule_Names();
                auto table_builder = LuaTableBuilder::Push_New_Table(engine);

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

                RulesLuaAdapter::Push_Rule_Type(engine, R::Sections(), section, key);

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

                RulesLuaAdapter::Push_Rule_Value(engine, R::Sections(), section, key);

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

                RulesLuaAdapter::Set_Rule_Value(engine, arguments, R::Editable_Sections(), section, key);

                return 1;
            });
        });
    }

protected:
    constexpr const char* Get_Cpp_Source() const override
    {
        return __FILE__;
    }
};
