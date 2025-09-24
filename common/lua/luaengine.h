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
#include <vector>
#include <format>

#include <lua.hpp>
#include <LuaBridge/LuaBridge.h>

#include "../logger.h"
#include "../paths.h"
#include "../twowaymap.h"

#include "luaresult.h"

using LuaVariant = std::variant<int, float, double, bool, std::string>;

template<typename T>
concept LuaVariantCompatible = (
    std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, double> || std::is_same_v<T, bool> || std::is_same_v<T, std::string>
);

template<typename T>
concept LuaPushType = (
    std::is_same_v<T, char*> ||
    std::is_same_v<T, const char*> ||
    std::is_same_v<T, std::string_view> ||
    std::is_same_v<T, std::string_view&> ||
    std::is_same_v<T, std::string> ||
    std::is_same_v<T, std::string&> ||
    std::is_same_v<T, double> ||
    std::is_same_v<T, float> ||
    std::is_same_v<T, int> ||
    std::is_same_v<T, bool>
);

/**
 * Avoids a circular dependency whilst enforcing
 * type constraints for LuaApi types.
 */
template <typename T, typename U>
concept LuaApiConcept = requires(T t, U& e)
{
    { t.Register(e) } -> std::same_as<void>;
    { t.Name } -> std::same_as<const std::string_view&>;
};

/**
 * Abstract wrapper around a Lua state.
 */
class LuaEngine
{
public:
    // all APIs will be available from this global Lua table
    inline static const std::string_view RootApiNamespace = "__CNC_API";
    inline static const TwoWayMap<int, std::string_view> LuaTypeMap {
        {LUA_TNONE, "none"},
        {LUA_TNIL, "nil"},
        {LUA_TBOOLEAN, "boolean"},
        {LUA_TLIGHTUSERDATA, "lightuserdata"},
        {LUA_TNUMBER, "number"},
        {LUA_TSTRING, "string"},
        {LUA_TTABLE, "table"},
        {LUA_TFUNCTION, "function"},
        {LUA_TUSERDATA, "userdata"},
        {LUA_TTHREAD, "thread"}
    };

    template<LuaVariantCompatible T>
    static std::string_view Get_Type_Name_For_Variant_Compatible()
    {
        if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double> || std::is_same_v<T, float>) {
            return LuaTypeMap[LUA_TNUMBER].value();
        } else if constexpr (std::is_same_v<T, bool>) {
            return LuaTypeMap[LUA_TBOOLEAN].value();
        } else if constexpr (std::is_same_v<T, std::string>) {
            return LuaTypeMap[LUA_TSTRING].value();
        }
    }

    // default path for lua script files - we ensure this is in the Lua 'package.path', see UniqueLuaEngine()
    static const std::filesystem::path& Get_Lua_Path()
    {
        static const auto lua_path = std::filesystem::path(Paths.Program_Path()) / "lua";

        return lua_path;
    }

    virtual ~LuaEngine() = default;

    #pragma region API

    template<class T, typename... Args>
    requires LuaApiConcept<T, LuaEngine>
    void Register_Api(Args&&... args)
    {
        auto api = T(std::forward<Args>(args)...);

        auto it = std::find(RegisteredApis.begin(), RegisteredApis.end(), api.Name);

        if (it != RegisteredApis.end()) {
            CNC_LOG_DEBUG("Request to register Api '{}' ignored, it's already registered", api.Name);
            return;
        }

        api.Register(*this);

        RegisteredApis.emplace_back(api.Name);
    }

    luabridge::Namespace Get_Api_Namespace(std::string_view name) const
    {
        return Bridge()
          .beginNamespace(RootApiNamespace.data())
          .beginNamespace(name.data());
    }

    void With_Api_Namespace(std::string_view name, std::function<void(luabridge::Namespace&)> action) const
    {
        auto api_namespace = Get_Api_Namespace(name);

        action(api_namespace);

        api_namespace.endNamespace().endNamespace();
    }

    #pragma endregion

    #pragma region State Access

    void With_State(std::function<void(lua_State*)> actions) const
    {
        actions(Get_State());
    }

    template<class T>
    T Get_Value_From_State(std::function<T(lua_State*)> actions) const
    {
        return actions(Get_State());
    }

    /**
     * Note: Calling this will terminate execution of the source CFunction
     * context, so no need to return after calling this.
     */
    void Raise_Error(const std::string& message) const
    {
        With_State([&](auto L) {
            Push_Value(message);
            lua_error(L);
        });
    }

    template<typename... Args>
    void Raise_Error_Format(const std::string& message, Args&&... args) const
    {
        Raise_Error(
            std::vformat(message, std::make_format_args(args...))
        );
    };

    #pragma endregion

    #pragma region Code Exec

    LuaResult Exec(const std::string& script) const
    {
        CNC_LOGGER_TRACE("Attempting to execute lua script: {}", script);

        return Get_Value_From_State<LuaResult>([&script](auto L)
        {
            auto status = luaL_loadstring(L, script.c_str());

            if (status != LUA_OK) {
                auto result = LuaResult(L, status);

                CNC_LOGGER_TRACE(
                    "Error loading lua script due to '{}' error: {}",
                    result.Code_As_String(),
                    result.Error_Message()
                );
                return result;
            }

            return LuaResult(
                L,
                lua_pcall(L, 0, LUA_MULTRET, 0)
            );
        });
    }

    std::future<LuaResult> Exec_Async(const std::string& script) const
    {
        auto promise = std::make_shared<std::promise<LuaResult>>();
        auto future = promise->get_future();

        std::thread([=]() {
            auto result = Exec(script);

            // TODO: Idea - fire a popup/show message event, have some hook to report errors for game engine to extend
            //       , user would get a nice popup/message with the lua error (we could have a lua debug rule to control this)
            if (!result.Is_Ok()) {
                CNC_LOGGER_ERROR(
                    "Error from background lua script: {} | script: {}",
                    result.Error_Message(),
                    script
                );
            }

            promise->set_value(result);
        }).detach();

        return future;
    }

    std::filesystem::path Resolve_Script_Path(std::filesystem::path script_path) const
    {
        auto full_script_path = script_path;

        if (script_path.is_relative()){
            // assume relative paths are part of @var{Lua_Path} file tree
            full_script_path = Get_Lua_Path() / script_path;
        }

        return full_script_path;
    }

    LuaResult Exec_File(std::filesystem::path script_path) const
    {        
        auto full_script_path = Resolve_Script_Path(script_path);

        CNC_LOGGER_DEBUG("Attempting to execute lua file: {}", full_script_path.string());

        return Get_Value_From_State<LuaResult>([&full_script_path](auto L) {
            auto status = luaL_loadfile(L, full_script_path.string().c_str());

            if (status != LUA_OK) {
                return LuaResult(L, status);
                auto result = LuaResult(L, status);

                CNC_LOGGER_TRACE(
                    "Error loading lua file '{}' due to error: {}",
                    full_script_path.string(),
                    result.Error_Message()
                );
                return result;
            }

            return LuaResult(
                L,
                lua_pcall(L, 0, LUA_MULTRET, 0)
            );
        });
    }

    LuaResult Exec_File_If_Exists(std::filesystem::path script_path) const
    {
        auto full_script_path = Resolve_Script_Path(script_path);

        if (!std::filesystem::is_regular_file(full_script_path)) {
            CNC_LOGGER_WARN("Skipping lua file execution as it does not exist: {}", full_script_path.string());
            return LuaResult(LUA_OK);
        }

        return Exec_File(full_script_path);
    }

    std::future<LuaResult> Exec_File_Async(std::filesystem::path script_path) const
    {
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
                    result.Error_Message(),
                    script_path.string()
                );
            }

            promise->set_value(result);
        }).detach();

        return future;
    }

    LuaResult PCall(std::string_view expression) const
    {
        if (!Is_Function()) {
            return LuaResult(
                std::format(
                    "Unable to call '{}' as it is either undefined or not a function",
                    expression
                )
            );
        }

        return Get_Value_From_State<LuaResult>([](auto L) {
            return LuaResult(
                L,
                lua_pcall(L, 0, LUA_MULTRET, 0)
            );
        });
    }

    template<LuaPushType... Args>
    LuaResult PCall_With_Args(std::string_view expression, Args&&... args) const
    {
        if (!Is_Function()) {
            return LuaResult(
                std::format(
                    "Unable to call '{}' as it is either undefined or not a function",
                    expression
                )
            );
        }

        Push_Values(std::forward<Args>(args)...);

        return Get_Value_From_State<LuaResult>([](auto L) {
            return LuaResult(
                L,
                lua_pcall(L, sizeof...(Args), LUA_MULTRET, 0)
            );
        });
    }

    #pragma endregion

    #pragma region Stack

    int Get_Stack_Count() const
    {
        return lua_gettop(Get_State());
    }

    template<LuaVariantCompatible T>
    bool Is_Type(int stack_index = -1) const
    {
        return Get_Value_From_State<bool>([&](auto L) {
            if constexpr (std::is_same_v<T, int>) {
                return lua_isinteger(L, stack_index);
            } else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>) {
                return lua_type(L, stack_index) == LUA_TNUMBER;
            } else if constexpr (std::is_same_v<T, bool>) {
                return lua_type(L, stack_index) == LUA_TBOOLEAN;
            } else if constexpr (std::is_same_v<T, std::string>) {
                return lua_type(L, stack_index) == LUA_TSTRING;
            }
        });
    }

    int Get_Lua_Type_Code(int stack_index = -1) const
    {
        return Get_Value_From_State<bool>([&](auto L) {
            return lua_type(L, stack_index);
        });
    }

    const std::string_view Get_Lua_Type(int stack_index = -1) const
    {
        auto type_code = Get_Lua_Type_Code(stack_index);

        return LuaTypeMap[type_code].value();
    }

    bool Is_Nil(int stack_index = -1) const
    {
        return Get_Value_From_State<bool>([&](auto L) {
            return lua_type(L, stack_index) == LUA_TNIL;
        });
    }

    bool Is_None(int stack_index = -1) const
    {
        return Get_Value_From_State<bool>([&](auto L) {
            return lua_type(L, stack_index) == LUA_TNONE;
        });
    }

    bool Is_Table(int stack_index = -1) const
    {
        return Get_Value_From_State<bool>([&](auto L) {
            return lua_istable(L, stack_index);
        });
    }

    bool Is_Function(int stack_index = -1) const
    {
        return Get_Value_From_State<bool>([&](auto L) {
            return lua_isfunction(L, stack_index);
        });
    }

    int Load_Global(std::string_view name) const
    {
        return Get_Value_From_State<int>([&](auto L) {
            return lua_getglobal(L, name.data());
        });
    }

    LuaResult With_Global(
        std::string_view name,
        int expected_lua_type,
        std::function<LuaResult()> action
    ) const
    {
        auto type = Load_Global(name);

        if (type != expected_lua_type) {
            Pop();
            return LuaResult(
                std::format(
                    "Global '{}' was of unexpected type '{}', expected '{}'",
                    name,
                    LuaTypeMap[type].value(),
                    LuaTypeMap[expected_lua_type].value()
                )
            );
        }

        auto result = action();

        Pop();

        return result;
    }

    int Load_Table_Field(std::string_view name, int stack_index = -1) const
    {
        return Get_Value_From_State<int>([&](auto L) {
            return lua_getfield(L, stack_index, name.data());
        });
    }

    LuaResult With_Table_Field(
        std::string_view parent_expression,
        std::string_view name,
        int expected_lua_type,
        std::function<LuaResult()> action
    ) const
    {
        auto type = Load_Table_Field(name);

        if (type != expected_lua_type) {
            Pop();
            return LuaResult(
                std::format(
                    "Table field '{}.{}' was of unexpected type '{}', expected '{}'",
                    parent_expression,
                    name,
                    LuaTypeMap[type].value(),
                    LuaTypeMap[expected_lua_type].value()
                )
            );
        }

        auto result = action();
        Pop();

        return result;
    }

    /**
     * Read a value from the stack, with type checking.
     */
    template<LuaVariantCompatible T>
    LuaResultWithValue<T> Try_Read(int stack_index = -1) const
    {
        return Get_Value_From_State<LuaResultWithValue<T>>([&](auto L) {
            if (!Is_Type<T>(stack_index)) {
                return LuaResultWithValue<T>(
                    L,
                    std::format(
                        "Failed to read from stack index {} due to type mismatch, wanted '{}' but stack value is: {}",
                        stack_index,
                        Get_Type_Name_For_Variant_Compatible<T>(),
                        Get_Lua_Type(stack_index)
                    )
                );
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
                return LuaResultWithValue<T>(
                    std::string(lua_tostring(L, stack_index))
                );
            }
        });
    }

    LuaResultWithValue<LuaVariant> Try_Read_Variant(int stack_index = -1) const
    {
        return Get_Value_From_State<LuaResultWithValue<LuaVariant>>([&](auto L) {
            std::optional<LuaVariant> variant_value;

            if (Is_Type<int>(stack_index)) {
                variant_value = Try_Read<int>(stack_index).Unpack();
            } else if (Is_Type<double>(stack_index)) {
                variant_value = Try_Read<double>(stack_index).Unpack();
            } else if (Is_Type<std::string>(stack_index)) {
                variant_value = Try_Read<std::string>(stack_index).Unpack();
            }

            if (variant_value.has_value()) {
                return LuaResultWithValue<LuaVariant>(variant_value.value());
            }

            return LuaResultWithValue<LuaVariant>(
                Get_State(),
                std::format("Attempted to read Lua value as variant, but type not supported by variant: {}", Get_Lua_Type(stack_index))
            );
        });
    }

    const std::string_view Get_Variant_Type(const LuaVariant& lua_variant) const
    {
        if (const auto value = std::get_if<std::string>(&lua_variant)) {
            return LuaTypeMap[LUA_TSTRING].value();
        } else if (const auto value = std::get_if<int>(&lua_variant)) {
            return LuaTypeMap[LUA_TNUMBER].value();
        } else if (const auto value = std::get_if<double>(&lua_variant)) {
            return LuaTypeMap[LUA_TNUMBER].value();
        } else if (const auto value = std::get_if<bool>(&lua_variant)) {
            return LuaTypeMap[LUA_TBOOLEAN].value();
        } else {
            CNC_LOG_FATAL("Attempted to get type for unsupported LuaVariant type");
        }
    }

    template<LuaVariantCompatible T>
    LuaResultWithValue<T> Try_Read_Table_Field(
        std::string_view parent_expression,
        std::string_view name
    ) const
    {
        auto expected_type = LUA_TNONE;
        
        if constexpr (std::is_same_v<T, bool>) {
            expected_type = LUA_TBOOLEAN;
        } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, double>) {
            expected_type = LUA_TNUMBER;
        } else if constexpr (std::is_same_v<T, std::string>) {
            expected_type = LUA_TSTRING;
        }

        LuaResultWithValue<T> value_result = LuaResult(LUA_OK);

        auto table_result = With_Table_Field(parent_expression, name, expected_type, [&]() {
            value_result = Try_Read<T>();

            return value_result;
        });

        if (!table_result.Is_Ok()) {
            return table_result;
        }

        return value_result;
    }

    LuaResultWithValue<std::string> To_String(int stack_index = -1) const
    {
        return Get_Value_From_State<LuaResultWithValue<std::string>>([&](auto L) {
            return LuaResultWithValue<std::string>(
                std::string(lua_tostring(L, stack_index))
            );
        });
    }

    void Pop(int amount = 1) const
    {
        With_State([&](auto L) {
            lua_pop(L, amount);
        });
    }

    template<LuaPushType T>
    void Push_Value(T value) const
    {
        With_State([&value](auto L) {
            if constexpr (std::is_same_v<T, char*> || std::is_same_v<T, const char*>) {
                lua_pushstring(L, value);
            } else if constexpr (std::is_same_v<T, std::string_view> || std::is_same_v<T, std::string_view&>) {
                lua_pushstring(L, value.data());
            } else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string&>) {
                lua_pushstring(L, value.c_str());
            } else if constexpr (std::is_same_v<T, double>) {
                lua_pushnumber(L, value);
            } else if constexpr (std::is_same_v<T, float>) {
                lua_pushnumber(L, (double)value);
            } else if constexpr (std::is_same_v<T, int>) {
                lua_pushinteger(L, value);
            } else if constexpr (std::is_same_v<T, bool>) {
                lua_pushboolean(L, value);
            }
        });
    }

    template<LuaPushType... Args>
    void Push_Values(Args&&... args) const
    {
        ((Push_Value(args)), ...);
    }

    void Push_Nil() const
    {
        With_State([](auto L){ lua_pushnil(L); });
    }

    bool Iterate_Over_Table(int stack_index = -1) const
    {
       return Get_Value_From_State<bool>([&](auto L){ return lua_next(L, stack_index) != 0; });
    }

    template<LuaPushType T>
    LuaResult Set_Table_Field(
        std::string_view table_expression,
        std::string_view name,
        T value,
        int table_stack_index = -1
    ) const
    {
        if (!Is_Table(table_stack_index)) {
            return LuaResult(
                std::format(
                    "Unable to set field '{}' as target '{}' is not a table",
                    name,
                    table_expression
                )  
            );
        }

        Push_Value(value);

        With_State([&](auto L) {
            lua_setfield(L, table_stack_index - 1, name.data());
        });

        return LuaResult(LUA_OK);
    }

    #pragma endregion

    #pragma region Expressions

    template<LuaVariantCompatible T>
    LuaResultWithValue<T> Eval(const std::string& expression) const
    {
        auto result = Exec(std::format("return {}", expression));
    
        if (!result.Is_Ok()) {
            return LuaResultWithValue<T>(
                result
            );
        }

        return Try_Read<T>();
    }

    template<LuaVariantCompatible T>
    std::future<LuaResultWithValue<T>> Eval_Async(const std::string& expression) const
    {
        auto promise = std::make_shared<std::promise<LuaResultWithValue<T>>>();
        auto future = promise->get_future();

        std::thread([=]() {
            promise->set_value(
                Eval<T>(expression)
            );
        }).detach();

        return future;
    }

    #pragma endregion

    // for API building
    luabridge::Namespace Bridge() const
    {
        return luabridge::getGlobalNamespace(Get_State());
    }
protected:
    inline static const CncLogger Logger = CncLogger("LuaEngine");

    std::vector<std::string_view> RegisteredApis;

    virtual lua_State* Get_State() const = 0;
};

/**
 * Smart pointer helper class to teardown lua_State pointers.
 */
class LuaStateDeleter
{
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
class UniqueLuaEngine : public LuaEngine
{
public:
    /**
     * Global lua state for the lifetime of the C++ process.
     * 
     * Note: This state is NOT shared or accessible from instances of UniqueLuaEngine.
     */
    static const UniqueLuaEngine& Global()
    {
        static UniqueLuaEngine global;

        return global;
    };

    UniqueLuaEngine() : State(Build_State(), LuaStateDeleter())
    {
        With_Global("package", LUA_TTABLE, [&]() {
            auto read_result = Try_Read_Table_Field<std::string>("package", "path");

            if (!read_result.Is_Ok() || !read_result.Has_Value()) {
                return (LuaResult)read_result;
            }

            return read_result.Map<LuaResult>([&](auto base_package_path) {
                auto package_path = std::format(
                    "{};{}/?.lua;{}/?/init.lua",
                    base_package_path,
                    Get_Lua_Path().string(),
                    Get_Lua_Path().string()
                );

                return Set_Table_Field("package", "path", package_path);
            });
        }).On_Error([](auto& r) {
            CNC_LOGGER_FATAL(
                "Failed to initialise Lua package paths: {}",
                r.Error_Message()
            );
        });
    }

protected:
    virtual lua_State* Get_State() const override
    {
        return State.get();
    }

private:
    std::unique_ptr<lua_State, LuaStateDeleter> State;

    static lua_State* Build_State()
    {
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
class SharedLuaEngine : public LuaEngine
{
public:
    SharedLuaEngine(lua_State* L) : State(L) {}

protected:
    virtual lua_State* Get_State() const override
    {
        return State;
    }

private:
    lua_State* State;
};

/**
 * Builder that makes registering APIs easier.
 */
template<class T>
class LuaEngineBuilder
{
public:
    template<class U, typename... Args>
    requires LuaApiConcept<U, LuaEngine>
    LuaEngineBuilder& With_Api(Args&&... args)
    {
        Lua.template Register_Api<U>(std::forward<Args>(args)...);

        return *this;
    }

    /**
     * Resolve the end of the builder chain as a concrete implementation.
     */
    T Build()
    {
        return std::move(Lua);
    }

private:
    T Lua;
};
