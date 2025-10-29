#include "luatablebuilder.h"

LuaTableBuilder& LuaTableBuilder::New_Table()
{
    Lua.With_State([](auto L) { lua_newtable(L); });

    TableStreamIndex = 1;

    return *this;
}
