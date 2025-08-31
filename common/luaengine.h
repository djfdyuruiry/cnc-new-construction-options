#pragma once

#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <format>

#include <lua.hpp>
#include <LuaBridge/LuaBridge.h>

#include "logger.h"

/**
 * C++ API for working with Lua. Uses LuaBridge to provide a
 * fluent interface for declaring classes, functions and variables.
 */

/**
 * Models the result of making a call to the low-level C Lua API.
 * 
 * Uses callbacks for fluent consumption: If_Ok and On_Error.
 */
class LuaResult {
public:
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
            Error = std::make_optional(
                std::string(lua_tostring(L, -1))
            );

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

private:
    int Lua_Code;
    std::optional<std::string> Error;
};

/**
 * Models the reading a value using the low-level C Lua API.
 * 
 * Use LuaResult fluent callbacks plus If_Value to read the value if set.
 */
template<class T>
class LuaResultWithValue : public LuaResult {
public:
    std::optional<T> value;

    LuaResultWithValue(const LuaResult& result) : LuaResult(result) {}

    LuaResultWithValue(T lua_value) : LuaResult(LUA_OK) {
        value = std::make_optional<T>(lua_value);
    }

    // L param is to prevent conflicts when T == std::string
    LuaResultWithValue(lua_State* L, std::string error) : LuaResult(error) {}

    const LuaResultWithValue& If_Value(std::function<void(T)> action) const {
        if (value.has_value()) {
            action(value.value());
        }

        return *this;
    }
};

/**
 * Abstract wrapper around a Lua state.
 */
class LuaEngine {
public:
    using LuaType = std::variant<std::string_view, double, int>;

    virtual ~LuaEngine() = default;

    void With_State(std::function<void(lua_State*)> actions) {
        actions(Get_State());
    }

    template<class T>
    T Get_Value_From_State(std::function<T(lua_State*)> actions) {
        return actions(Get_State());
    }

    LuaResult Exec(const std::string& script) {
        CNC_LOGGER_TRACE("Attempting to execute lua script: {}", script);

        return Get_Value_From_State<LuaResult>([&script](auto L) {
            auto status = luaL_loadstring(L, script.data());

            if (status != LUA_OK) {
                return LuaResult(L, status);
            }

            return LuaResult(
                L,
                lua_pcall(L, 0, LUA_MULTRET, 0)
            );
        });
    }

    LuaResult Exec_File(std::string_view script_path) {
        CNC_LOGGER_DEBUG("Attempting to execute lua file: {}", script_path);

        return Get_Value_From_State<LuaResult>([&script_path](auto L) {
            auto status = luaL_loadfile(L, script_path.data());

            if (status != LUA_OK) {
                return LuaResult(L, status);
            }

            return LuaResult(
                L,
                lua_pcall(L, 0, LUA_MULTRET, 0)
            );
        });
    };

    template<class T>
    void Push_Value(T value) {
        With_State([&value](auto L) {
            if constexpr (std::is_same_v<T, std::string_view> || std::is_same_v<T, std::string>) {
                lua_pushstring(L, value.data());
            } else if constexpr (std::is_same_v<T, double>) {
                lua_pushnumber(L, value);
            } else if constexpr (std::is_same_v<T, int>) {
                lua_pushinteger(L, value);
            } else {
                CNC_LOGGER_FATAL("Attempted to write unsupported Lua type");
            }
        });
    }

    LuaResultWithValue<std::string> To_String(int stack_index = -1) {
        return Get_Value_From_State<LuaResultWithValue<std::string>>([&stack_index](auto L) {
            return LuaResultWithValue<std::string>(
                lua_tostring(L, stack_index)
            );
        });
    }

    /**
     * Read a value from the stack, with type checking.
     */
    template<class T>
    LuaResultWithValue<T> Try_Read(int stack_index = -1) {
        return Get_Value_From_State<LuaResultWithValue<T>>([&stack_index](auto L) {
            auto type = lua_type(L, stack_index);

            if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double>) {
                if (type != LUA_TNUMBER) {
                    return LuaResultWithValue<T>(
                        L,
                        std::format(
                            "Failed to read int/number from stack index {} due to type mismatch, actual type: {}",
                            stack_index,
                            type
                        )
                    );
                }
            }

            if constexpr (std::is_same_v<T, int>) {
                return LuaResultWithValue<T>(
                    lua_tointeger(L, stack_index)
                );
            } else if constexpr (std::is_same_v<T, double>) {
                return LuaResultWithValue<T>(
                    lua_tonumber(L, stack_index)
                );
            } else if constexpr (std::is_same_v<T, std::string>) {
                if (type != LUA_TSTRING) { 
                    return LuaResultWithValue<T>(
                        L,
                        std::format(
                            "Failed to read string from stack index {} due to type mismatch, actual type: {}",
                            stack_index,
                            type
                        )
                    );
                }

                return LuaResultWithValue<T>(
                    std::string(lua_tostring(L, stack_index))
                );
            }

            CNC_LOGGER_FATAL("Attempted to read unsupported Lua type");
        });
    }

    template<class T>
    LuaResultWithValue<T> Eval(const std::string& expression) {
        auto result = Exec(std::format("return {}", expression));
    
        if (!result.Is_Ok()) {
            return LuaResultWithValue<T>(
                result
            );
        }

        return Try_Read<T>();
    }

    luabridge::Namespace Bridge() {
        return luabridge::getGlobalNamespace(Get_State());
    }

protected:
    inline static const CncLogger Logger = CncLogger("LuaEngine");

    virtual lua_State* Get_State() = 0;
};

/**
 * Smart pointer helper class to teardown lua_State pointers.
 */
class LuaStateDeleter {
public:
    void operator()(lua_State *L) const
    {
        if (L)
        {
            lua_close(L);
        }
    }
};

/**
 * Instances of this LuaEngine should be created for the lifecycle of a
 * given context to ensure clean state when starting a new context
 * (scenario/screen/thread etc.).
 * 
 * Aligns with smart pointer unique logic.
 */
class UniqueLuaEngine : public LuaEngine {
public:
    // for storing persistent Lua state globally
    static const UniqueLuaEngine& Global() {
        static UniqueLuaEngine global;

        return global;
    };

    UniqueLuaEngine() : State(Build_State(), LuaStateDeleter()) {
    }

protected:
    virtual lua_State* Get_State() {
        return State.get();
    }

private:
    std::unique_ptr<lua_State, LuaStateDeleter> State;

    static lua_State* Build_State() {
        auto L = luaL_newstate();
        luaL_openlibs(L);

        return L;
    };
};

/**
 * Instances of this LuaEngine should be created to wrap around
 * a state that isn't owned by the current context, e.x. in a
 * Lua C function.
 * 
 * Aligns with smart pointer shared logic.
 */
class SharedLuaEngine : public LuaEngine {
public:
    SharedLuaEngine(lua_State* L) : State(L) {
    }

protected:
    virtual lua_State* Get_State() {
        return State;
    }

private:
    lua_State* State;
};
