#pragma once

#include "luaengine.h"
#include "lualib.h"

/**
 * Fluent builder for setting up Lua Tables on the stack.
 * Supports both index based array tables and key=value tables.
 * 
 * Arrays: Push_New_Table(...) -> With_Index_Value() (repeat With_Index_Value as needed)
 * Maps:   Push_New_Table(...) -> With_Key_Value()   (repeat With_Key_Value as needed)
 * 
 * Values are pushed onto the stack as methods are called, no need to finalize. If no methods
 * are called after Push_New_Table then the stack will contain an empty table.
 */
class LuaTableBuilder
{
public:
    static LuaTableBuilder Push_New_Table(const LuaEngine& engine);

    template <LuaPushType T>
    LuaTableBuilder& With_Index_Value(T value)
    {
        Lua.Push_Value<T>(value);
        Lua.With_State([&](auto L) {
            lua_rawseti(L, -2, TableStreamIndex);
        });
    
        TableStreamIndex = TableStreamIndex + 1;

        return *this;
    }

    template <LuaPushType T>
    LuaTableBuilder& With_Key_Value(std::string_view key, T value)
    {
        Lua.Push_Value<std::string_view>(key);
        Lua.Push_Value<T>(value);
        Lua.With_State([](auto L) { lua_settable(L, -3); });

        TableStreamIndex = TableStreamIndex + 1;

        return *this;
    }
private:
    const LuaEngine& Lua;

    unsigned int TableStreamIndex;

    LuaTableBuilder(const LuaEngine& engine);
};
