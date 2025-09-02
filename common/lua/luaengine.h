#pragma once

/**
 * C++ API for working with Lua. Uses LuaBridge to provide a
 * fluent interface for declaring classes, functions and variables.
 * 
 * Class hierarchy:
 * 
 *   LuaEngine --[uses]--> LuaResult --[uses]--> LuaResultWithValue
 *   LuaArguments --[uses]--> LuaEngine
 *   UniqueLuaEngine --[extends]--> LuaEngine
 *      UniqueLuaEngine --[uses]--> LuaStateDeleter
 *   LuaEngineBuilder --[builds]--> UniqueLuaEngine
 *   SharedLuaEngine --[extends]--> LuaEngine
 */

#include <filesystem>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <variant>
#include <format>

#include <lua.hpp>
#include <LuaBridge/LuaBridge.h>

#include "../logger.h"
#include "luaresult.h"

/**
 * Abstract wrapper around a Lua state.
 */
class LuaEngine {
public:
    inline static const std::filesystem::path Lua_Directory = std::filesystem::path(Paths.Program_Path()) / "lua";

    virtual ~LuaEngine() = default;

    template<class T, typename... Args>
    void Register_Api(Args&&... args) {
        auto api = T(*this, std::forward<Args>(args)...);

        api.Register();
    }

    // low level state access

    void With_State(std::function<void(lua_State*)> actions) const {
        actions(Get_State());
    }

    template<class T>
    T Get_Value_From_State(std::function<T(lua_State*)> actions) const {
        return actions(Get_State());
    }

    /**
     * Note: Calling this will terminate execution of the source CFunction
     * context, so no need to return after calling this.
     */
    void Raise_Error(const std::string& message) const {
        With_State([&](auto L) {
            Push_Value(message);
            lua_error(L);
        });
    }

    // code execution

    LuaResult Exec(const std::string& script) const {
        CNC_LOGGER_TRACE("Attempting to execute lua script: {}", script);

        return Get_Value_From_State<LuaResult>([&script](auto L) {
            auto status = luaL_loadstring(L, script.c_str());

            if (status != LUA_OK) {
                auto result = LuaResult(L, status);

                CNC_LOGGER_TRACE(
                    "Error loading lua script due to '{}' error: {}",
                    result.Code_As_String(),
                    result.Error.value()
                );
                return result;
            }

            return LuaResult(
                L,
                lua_pcall(L, 0, LUA_MULTRET, 0)
            );
        });
    }

    std::future<LuaResult> Exec_Async(const std::string& script) const {
        auto promise = std::make_shared<std::promise<LuaResult>>();
        auto future = promise->get_future();

        std::thread([=]() {
            auto result = Exec(script);

            // TODO: Idea - fire a popup/show message event, have some hook to report errors for game engine to extend
            //       , user would get a nice popup/message with the lua error (we could have a lua debug rule to control this)
            if (!result.Is_Ok()) {
                CNC_LOGGER_ERROR(
                    "Error from background lua script: {} | script: {}",
                    result.Error.value(),
                    script
                );
            }

            promise->set_value(result);
        }).detach();

        return future;
    }

    LuaResult Exec_File(std::string_view script_path) const {
        CNC_LOGGER_DEBUG("Attempting to execute lua file: {}", script_path);

        return Get_Value_From_State<LuaResult>([&script_path](auto L) {
            auto status = luaL_loadfile(L, script_path.data());

            if (status != LUA_OK) {
                return LuaResult(L, status);
                auto result = LuaResult(L, status);

                CNC_LOGGER_TRACE(
                    "Error loading lua file due to '{}' error: {}",
                    result.Code_As_String(),
                    result.Error.value()
                );
                return result;
            }

            return LuaResult(
                L,
                lua_pcall(L, 0, LUA_MULTRET, 0)
            );
        });
    };

    std::future<LuaResult> Exec_File_Async(std::string_view script_path) const {
        auto promise = std::make_shared<std::promise<LuaResult>>();
        auto future = promise->get_future();

        std::thread([=]() {
            auto result = Exec_File(script_path);

            // TODO: Idea - fire a popup/show message event, have some hook to report errors for game engine to extend
            //       , user would get a nice popup/message with the lua error (we could have a lua debug rule to control this)
            if (!result.Is_Ok()) {
                // TODO: output debug info - maybe have a method in LuaEvent that builds a standard error message for logging or logs directly
                CNC_LOGGER_ERROR(
                    "Error from background lua script file: {} | script_path: {}",
                    result.Error.value(),
                    script_path
                );
            }

            promise->set_value(result);
        }).detach();

        return future;
    }

    // lua stack interaction (inspect, read and write values)

    int Get_Stack_Count() const {
        return lua_gettop(Get_State());
    }

    template<class T>
    bool Is_Type(int stack_index = -1) const {
        return Get_Value_From_State<bool>([&stack_index](auto L) {
            auto type = lua_type(L, stack_index);

            if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double> || std::is_same_v<T, float>) {
                return type == LUA_TNUMBER;
            } else if constexpr (std::is_same_v<T, bool>) {
                return type == LUA_TBOOLEAN;
            } else if constexpr (std::is_same_v<T, std::string>) {
                return type == LUA_TSTRING;
            }

            return false;
        });
    }

    bool Is_Nil(int stack_index = -1) const {
        return Get_Value_From_State<bool>([&stack_index](auto L) {
            return lua_type(L, stack_index) == LUA_TNIL;
        });
    }

    bool Is_None(int stack_index = -1) const {
        return Get_Value_From_State<bool>([&stack_index](auto L) {
            return lua_type(L, stack_index) == LUA_TNONE;
        });
    }

    /**
     * Read a value from the stack, with type checking.
     */
    template<class T>
    LuaResultWithValue<T> Try_Read(int stack_index = -1) const {
        return Get_Value_From_State<LuaResultWithValue<T>>([&stack_index](auto L) {
            auto type = lua_type(L, stack_index);

            if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double> || std::is_same_v<T, float>) {
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
            } else if constexpr (std::is_same_v<T, float>) {
                return LuaResultWithValue<T>(
                    (float)lua_tonumber(L, stack_index)
                );
            } else if constexpr (std::is_same_v<T, bool>) {
                return LuaResultWithValue<T>(
                    lua_toboolean(L, stack_index)
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

    LuaResultWithValue<std::string> To_String(int stack_index = -1) const {
        return Get_Value_From_State<LuaResultWithValue<std::string>>([&stack_index](auto L) {
            return LuaResultWithValue<std::string>(
                lua_tostring(L, stack_index)
            );
        });
    }

    template<class T>
    void Push_Value(T value) const {
        With_State([&value](auto L) {
            if constexpr (std::is_same_v<T, std::string_view>) {
                lua_pushstring(L, value.data());
            } else if constexpr (std::is_same_v<T, std::string>) {
                lua_pushstring(L, value.c_str());
            } else if constexpr (std::is_same_v<T, double>) {
                lua_pushnumber(L, value);
            } else if constexpr (std::is_same_v<T, float>) {
                lua_pushnumber(L, (double)value);
            } else if constexpr (std::is_same_v<T, int>) {
                lua_pushinteger(L, value);
            } else if constexpr (std::is_same_v<T, bool>) {
                lua_pushboolean(L, value);
            } else {
                CNC_LOGGER_FATAL("Attempted to write unsupported C++ type");
            }
        });
    }

    // Read values using expressions

    template<class T>
    LuaResultWithValue<T> Eval(const std::string& expression) const {
        auto result = Exec(std::format("return {}", expression));
    
        if (!result.Is_Ok()) {
            return LuaResultWithValue<T>(
                result
            );
        }

        return Try_Read<T>();
    }

    template<class T>
    std::future<LuaResultWithValue<T>> Eval_Async(const std::string& expression) const {
        auto promise = std::make_shared<std::promise<LuaResultWithValue<T>>>();
        auto future = promise->get_future();

        std::thread([=]() {
            promise->set_value(
                Eval<T>(expression)
            );
        }).detach();

        return future;
    }

    // bridge for API building

    luabridge::Namespace Bridge() const {
        return luabridge::getGlobalNamespace(Get_State());
    }
protected:
    inline static const CncLogger Logger = CncLogger("LuaEngine");

    virtual lua_State* Get_State() const = 0;
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
        Exec(
            std::format(
                "package.path = package.path .. ';{}/?.lua;{}/?/init.lua'",
                Lua_Directory.string(),
                Lua_Directory.string()
            )
        ).On_Error([](auto& r) {
            CNC_LOGGER_CRITICAL("Failed to initialise Lua package paths: {}", r.Error.value());
        });
    }

protected:
    virtual lua_State* Get_State() const override {
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
 * Lua CFunction.
 * 
 * Aligns with smart pointer shared logic.
 */
class SharedLuaEngine : public LuaEngine {
public:
    SharedLuaEngine(lua_State* L) : State(L) {
    }

protected:
    virtual lua_State* Get_State() const override {
        return State;
    }

private:
    lua_State* State;
};

/**
 * Builder that makes registering APIs easier.
 */
template<class T>
class LuaEngineBuilder {
public:
    template<class U, typename... Args>
    LuaEngineBuilder& With_Api(Args&&... args) {
        Lua.template Register_Api<U>(std::forward<Args>(args)...);

        return *this;
    }

    /**
     * Resolve the end of the builder chain as a concrete implementation.
     */
    T Build() {
        return std::move(Lua);
    }
private:
    T Lua;
};
