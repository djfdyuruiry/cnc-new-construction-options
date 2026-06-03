#pragma once

#include "luaarguments.h"
#include "luaengine.h"
#include "rulesections.h"

class RulesLuaAdapter final
{
public:
    static void Push_Rule_Type(
        const LuaEngine& engine,
        const RuleSections& sections,
        std::string section,
        std::string key
    );

    /**
     * Unpack variant to call corresponding engine Push_Value template.
     */
    static void Push_Rule_Variant(const LuaEngine& engine, const RuleValueVariant& variant);

    static void Push_Rule_Value(
        const LuaEngine& engine,
        const RuleSections& sections,
        std::string section,
        std::string key
    );

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
        const RuleSections& sections,
        const std::string& section,
        const std::string& key
    );

    static void Assert_Rule_Exists(
        const LuaEngine& engine,
        const RuleSections& sections,
        std::string section,
        std::string key
    );

private:
    RulesLuaAdapter() = delete;
};
