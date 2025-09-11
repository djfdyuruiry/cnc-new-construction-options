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

        TableStreamIndex = 1;

        return *this;
    }

    template <class T>
    LuaTableBuilder& With_Index_Value(T value)
    {
        if (!TableStreamIndex.has_value()) {
            New_Table();
        }

        Lua.Push_Value<T>(value);
        Lua.With_State([&](auto L) {
            lua_rawseti(L, -2, TableStreamIndex.value());
        });
    
        TableStreamIndex = TableStreamIndex.value() + 1;

        return *this;
    }

    template <class T>
    LuaTableBuilder& With_Key_Value(std::string_view key, T value)
    {
        if (!TableStreamIndex.has_value()) {
            New_Table();
        }

        Lua.Push_Value<std::string_view>(key);
        Lua.Push_Value<T>(value);
        Lua.With_State([](auto L) { lua_settable(L, -3); });

        TableStreamIndex = TableStreamIndex.value() + 1;

        return *this;
    }
private:
    const LuaEngine& Lua;

    std::optional<int> TableStreamIndex;
};
