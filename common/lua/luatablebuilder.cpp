#include "luatablebuilder.h"

LuaTableBuilder LuaTableBuilder::Push_New_Table(const LuaEngine& engine)
{
    return { engine };
}

LuaTableBuilder::LuaTableBuilder(const LuaEngine& engine) : Lua(engine)
{
    Lua.With_State([](auto L) { lua_newtable(L); });

    TableStreamIndex = 1;
}
