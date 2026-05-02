#pragma once

#include <functional>
#include <optional>
#include <string>

#include "lualib.h"

#include "logger.h"
#include "twowaymap.h"

/**
 * Models the result of making a call to the low-level C Lua API.
 * 
 * Uses callbacks for fluent consumption: If_Ok and On_Error.
 */
class LuaResult
{
public:
    static inline const TwoWayMap<int, std::string_view> LuaErrorMap {
        { LUA_OK, "OK" },
        { LUA_ERRRUN, "Runtime" },
        { LUA_ERRSYNTAX, "Syntax" },
        { LUA_ERRMEM, "Memory" },
        { LUA_ERRFILE, "File" },
        { LUA_ERRERR, "Error Handling Failure" }
    };

    /**
     * If @code is not LUA_OK, this constructor has side effects and will
     * pop the last error off the Lua stack @L.
     */
    LuaResult(lua_State* L, int code) ;

    LuaResult(int code): LuaCode(code) {}

    /**
     * Provide a custom error message and set a custom error state. 
     */
    LuaResult(std::string error);

    bool Is_Ok() const;

    std::string_view Code_As_String() const;

    std::string Error_Message() const;

    const LuaResult& If_Ok(const std::function<void(const LuaResult&)>& action) const;

    const LuaResult& On_Error(const std::function<void(const LuaResult&)>& action) const;

    bool Has_Debug_Info() const;

    const std::optional<lua_Debug>& Debug_Info() const;

protected:
    int LuaCode;
    std::optional<std::string> Error;
    std::optional<lua_Debug> DebugInfo;
};

/**
 * Models the reading a value using the low-level C Lua API.
 * 
 * Use LuaResult fluent callbacks plus If_Value to read the value if set.
 */
template<class T>
class LuaResultWithValue : public LuaResult
{
public:
    LuaResultWithValue(const LuaResult& result) : LuaResult(result) {}

    LuaResultWithValue(T lua_value) : LuaResult(LUA_OK)
    {
        ValueSource = std::make_optional<T>(lua_value);
    }

    // L param is to prevent conflicts when T == std::string
    LuaResultWithValue(lua_State* L, std::string error) : LuaResult(error) {}

    const LuaResultWithValue& If_Value(std::function<void(T)> action) const
    {
        if (ValueSource.has_value()) {
            action(ValueSource.value());
        }

        return *this;
    }

    bool Has_Value()
    {
        return ValueSource.has_value();
    }

    /**
     * This method is dangerous, and must only be called after
     * checking Is_Ok and Has_Value both return `true`.
     */
    T Unpack()
    {
        if (!ValueSource.has_value()) {
            CNC_LOGGER_FATAL("Attempted to unpack empty result");
        }

        return ValueSource.value();
    }

    template<class U>
    U Map(std::function<U(T)> mapper)
    {
        if (!ValueSource.has_value()) {
            CNC_LOGGER_FATAL("Attempted to map empty result");
        }

        return mapper(ValueSource.value());
    }

private:
    static inline const auto& Logger = CncLogger::For(LuaResultWithValue);

    std::optional<T> ValueSource;
};


/**
 * Models the result of evaluating a lua expression.
 */
class LuaEvalResult : public LuaResult
{
public:
    LuaEvalResult(const LuaResult& result) : LuaResult(result), ReturnedValue(false) {}

    LuaEvalResult(const LuaResult& result, const bool eval_returned_value) : LuaResult(result)
    {
        ReturnedValue = eval_returned_value;
    }

    const LuaEvalResult& If_Value(const std::function<void()>& action) const
    {
        if (Returned_Value()) {
            action();
        }

        return *this;
    }

    bool Returned_Value() const
    {
        return ReturnedValue;
    }

private:
    static inline const auto& Logger = CncLogger::For(LuaResultWithValue);

    bool ReturnedValue;
};
