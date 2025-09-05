#pragma once

#include <functional>
#include <optional>
#include <string>

#include <lua.hpp>

#include "../logger.h"
#include "../twowaymap.h"

/**
 * Models the result of making a call to the low-level C Lua API.
 * 
 * Uses callbacks for fluent consumption: If_Ok and On_Error.
 */
class LuaResult {
public:
    inline static const TwoWayMap<int, std::string_view> Lua_Error_Map {
        { LUA_OK, "OK" },
        { LUA_ERRRUN, "Runtime" },
        { LUA_ERRSYNTAX, "Syntax" },
        { LUA_ERRMEM, "Memory" },
        { LUA_ERRFILE, "File" },
        { LUA_ERRERR, "Error Handling Failure" }
    };

    int Lua_Code;
    std::optional<std::string> Error;
    std::optional<lua_Debug> Debug_Info;

    /**
     * If @code is not LUA_OK, this constructor has side affects and will
     * pop the last error off the Lua stack @L.
     */
    LuaResult(lua_State* L, int code) : Lua_Code(code) {
        if (code == LUA_ERRERR + 1) {
            // custom lua error provided
            return;
        }

        if (code < LUA_OK || code > LUA_ERRERR) {
            // invalid error code passed, assume error
            code = LUA_ERRERR;
        }

        if (code != LUA_OK) {
            Error = std::string(lua_tostring(L, -1));

            lua_Debug debug;

            if (lua_getstack(L, 0, &debug)) {
                Debug_Info = debug;
            }

            lua_pop(L, 1);
        }
    }

    LuaResult(int code): Lua_Code(code) {}

    /**
     * Provide a custom error message and set a custom error state. 
     */
    LuaResult(std::string error) : Lua_Code(LUA_ERRERR + 1) {
        Error = std::make_optional(error);
    }

    bool Is_Ok() const {
        return Lua_Code == LUA_OK;
    }

    const std::string_view Code_As_String() const {
        return Lua_Error_Map[Lua_Code].value_or("Unknown");
    }

    const LuaResult& If_Ok(std::function<void(const LuaResult&)> action) const {
        if (Is_Ok()) {
            action(*this);
        }

        return *this;
    }

    const LuaResult& On_Error(std::function<void(const LuaResult&)> action) const {
        if (!Is_Ok()) {
            action(*this);
        }

        return *this;
    }
};

/**
 * Models the reading a value using the low-level C Lua API.
 * 
 * Use LuaResult fluent callbacks plus If_Value to read the value if set.
 */
template<class T>
class LuaResultWithValue : public LuaResult {
public:
    LuaResultWithValue(const LuaResult& result) : LuaResult(result) {}

    LuaResultWithValue(T lua_value) : LuaResult(LUA_OK) {
        Value_Source = std::make_optional<T>(lua_value);
    }

    // L param is to prevent conflicts when T == std::string
    LuaResultWithValue(lua_State* L, std::string error) : LuaResult(error) {}

    const LuaResultWithValue& If_Value(std::function<void(T)> action) const {
        if (Value_Source.has_value()) {
            action(Value_Source.value());
        }

        return *this;
    }

    bool Has_Value() {
        return Value_Source.has_value();
    }

    /**
     * This method is dangerous, and must only be called after
     * checking Is_Ok and Has_Value both return `true`.
     */
    T Unpack() {
        if (!Value_Source.has_value()) {
            CNC_LOG_FATAL("Attempted to unpack empty LuaResultWithValue");
        }

        return Value_Source.value();
    }

private:
    std::optional<T> Value_Source;
};
