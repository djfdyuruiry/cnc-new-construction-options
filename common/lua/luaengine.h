#pragma once

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <thread>
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
    int Lua_Code;
    std::optional<std::string> Error;

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

    const char * Code_As_String() const {
        switch (Lua_Code) {
            case LUA_OK:
                return "OK";
            case LUA_ERRRUN:
                return "Runtime";
            case LUA_ERRSYNTAX:
                return "Syntax";
            case LUA_ERRMEM:
                return "Memory";
            case LUA_ERRFILE:
                return "File";
            case LUA_ERRERR:
                return "Error Handling Failure";
            default:
                return "Unknown";
        }
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

/**
 * Abstract wrapper around a Lua state.
 */
class LuaEngine {
public:
    using LuaType = std::variant<std::string_view, double, int>;

    virtual ~LuaEngine() = default;

    template<class T>
    void Register_Api() {
        auto api = T(*this);

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
     * Note: Calling this will terminate execution of the source function/method
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
            promise->set_value(
                Exec(script)
            );
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
            promise->set_value(
                Exec_File(script_path)
            );
        }).detach();

        return future;
    }

    // lua stack interaction (read/write values)

    int Get_Stack_Count() const {
        return lua_gettop(Get_State());
    }

    /**
     * Read a value from the stack, with type checking.
     */
    template<class T>
    LuaResultWithValue<T> Try_Read(int stack_index = -1) const {
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

    LuaResultWithValue<std::string> To_String(int stack_index = -1) const {
        return Get_Value_From_State<LuaResultWithValue<std::string>>([&stack_index](auto L) {
            return LuaResultWithValue<std::string>(
                lua_tostring(L, stack_index)
            );
        });
    }

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

    template<class T>
    void Push_Value(T value) const {
        With_State([&value](auto L) {
            if constexpr (std::is_same_v<T, std::string_view>) {
                lua_pushstring(L, value.data());
            } else if constexpr (std::is_same_v<T, std::string>) {
                lua_pushstring(L, value.c_str());
            } else if constexpr (std::is_same_v<T, double>) {
                lua_pushnumber(L, value);
            } else if constexpr (std::is_same_v<T, int>) {
                lua_pushinteger(L, value);
            } else {
                CNC_LOGGER_FATAL("Attempted to write unsupported Lua type");
            }
        });
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
 * Wrapper around LuaEngine to validate and read arguments passed to a CFunction.
 */
class LuaArguments {
public:
    const int Count;
    const std::string Function_Signature;

    LuaArguments(const LuaEngine& lua, std::string function_signature) : 
        Lua(lua),
        Count(lua.Get_Stack_Count()),
        Function_Signature(function_signature) {}

    // Fluent assert stream methods

    LuaArguments& Count_Is(int expected) {
        if (Stream_Is_Valid.has_value() && !Stream_Is_Valid.value()) {
            // already invalid, short circuit
            return *this;
        }

        Stream_Is_Valid = Count == expected;
        Stream_Argument_Index = 1;

        return *this;
    }

    template<class T>
    LuaArguments& Next_Argument_Is() {
        if (Stream_Is_Valid.has_value() && !Stream_Is_Valid.value()) {
            // already invalid, short circuit
            return *this;
        }

        if (!Stream_Argument_Index.has_value()) {
            Stream_Argument_Index = 1;
        }

        auto result = Lua.Try_Read<T>(Stream_Argument_Index.value());

        Stream_Is_Valid = result.Is_Ok() && result.Has_Value();
        Stream_Argument_Index = Stream_Argument_Index.value() + 1;

        return *this;
    }

    template<class T>
    LuaArguments& First_Argument_Is() {
        Stream_Argument_Index = 1;

        return Next_Argument_Is<T>();
    }

    bool Assert() {
        if (!Stream_Is_Valid.has_value()) {
            // called before Is calls, assume invalid
            Stream_Is_Valid = false;
        }

        auto result = Stream_Is_Valid.value();

        if (!result) {
            Lua.Raise_Error(
                std::format(
                    "Incorrect number of arguments, or argument type mis-match. Usage: {}",
                    Function_Signature
                )
            );
        }

        Stream_Is_Valid.reset();
        Stream_Argument_Index.reset();

        return result;
    }

    // Fluent read stream methods

    template<class T>
    LuaResultWithValue<T> Read_Next() {
        if (!Read_Stream_Argument_Index.has_value()) {
            Read_Stream_Argument_Index = 1;
        }

        auto result = Lua.Try_Read<T>(Read_Stream_Argument_Index.value());

        Read_Stream_Argument_Index = Read_Stream_Argument_Index.value() + 1;

        return result;
    }

    template<class T>
    LuaResultWithValue<T> Read_First() {
        Read_Stream_Argument_Index = 1;

        return Read_Next<T>();
    }
private:
    const LuaEngine& Lua;
    std::optional<bool> Stream_Is_Valid;
    std::optional<int> Stream_Argument_Index;

    std::optional<int> Read_Stream_Argument_Index;
};

/**
 * Builder that makes registering APIs easier.
 */
template<class T>
class LuaEngineBuilder {
public:
    template<class U>
    LuaEngineBuilder& With_Api() {
        Lua.template Register_Api<U>();

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
 * Lua C function.
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
