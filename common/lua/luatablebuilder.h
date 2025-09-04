#pragma once

#include <optional>

#include <lua.hpp>

#include "luaengine.h"

/**
 * Fluent builder for setting up Lua Tables on the stack.
 * Supports both index based array tables and key=value tables.
 * 
 * Arrays: New_Table() -> With_Index_Value() (repeat With_Index_Value as needed)
 * Maps:   New_Table() -> With_Key_Value()   (repeat With_Key_Value as needed)
 * 
 * Values are pushed onto the stack as methods are called, no need to finalize.
 */
class LuaTableBuilder
{
public:
    LuaTableBuilder(const LuaEngine& engine) : Lua(engine) {}

    LuaTableBuilder& New_Table()
    {
        Lua.With_State([](auto L) { lua_newtable(L); });

        Table_Stream_Index = 1;

        return *this;
    }

    template <class T>
    LuaTableBuilder& With_Index_Value(T value)
    {
        if (!Table_Stream_Index.has_value()) {
            New_Table();
        }

        Lua.Push_Value<T>(value);
        Lua.With_State([&](auto L) {
            lua_rawseti(L, -2, Table_Stream_Index.value());
        });
    
        Table_Stream_Index = Table_Stream_Index.value() + 1;

        return *this;
    }

    template <class T>
    LuaTableBuilder& With_Key_Value(std::string_view key, T value)
    {
        if (!Table_Stream_Index.has_value()) {
            New_Table();
        }

        Lua.Push_Value<std::string_view>(key);
        Lua.Push_Value<T>(value);
        Lua.With_State([](auto L) { lua_settable(L, -3); });

        Table_Stream_Index = Table_Stream_Index.value() + 1;

        return *this;
    }
private:
    const LuaEngine& Lua;

    std::optional<int> Table_Stream_Index;
};
