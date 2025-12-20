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

#include "lualib.h"

#include "logger.h"
#include "twowaymap.h"
#include "luaresult.h"

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned char uchar;

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
    std::is_same_v<T, uint> ||
    std::is_same_v<T, bool> ||
    std::is_same_v<T, char> ||
    std::is_same_v<T, uchar> ||
    std::is_same_v<T, ushort>
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
    static constexpr std::string_view RootApiNamespace = "__CNC_API";
    static const TwoWayMap<int, std::string_view> LuaTypeMap;

    template<LuaVariantCompatible T>
    static std::string_view Get_Type_Name_For_Variant_Compatible()
    {
        if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double> || std::is_same_v<T, float>) {
            return LuaTypeMap[LUA_TNUMBER].value();
        } else if constexpr (std::is_same_v<T, bool>) {
            return LuaTypeMap[LUA_TBOOLEAN].value();
        } else if constexpr (std::is_same_v<T, std::string>) {
            return LuaTypeMap[LUA_TSTRING].value();
        } else {
            throw std::invalid_argument("Unsupported LuaVariant type - this is normally caused by variant type list being updated without updating supporting code");
        }
    }

    std::filesystem::path Resolve_Script_Path(const std::filesystem::path& script_path) const;

    virtual ~LuaEngine() = default;

    #pragma region API

    template<class T, typename... Args>
    requires LuaApiConcept<T, LuaEngine>
    void Register_Api(Args&&... args)
    {
        auto api = T(std::forward<Args>(args)...);

        auto it = std::find(RegisteredApis.begin(), RegisteredApis.end(), api.Name);

        if (it != RegisteredApis.end()) {
            CNC_LOGGER_DEBUG("Request to register Api '{}' ignored, it's already registered", api.Name);
            return;
        }

        api.Register(*this);

        RegisteredApis.emplace_back(api.Name);
    }

    luabridge::Namespace Get_Api_Namespace(const std::string_view name) const;

    void With_Api_Namespace(const std::string_view name, const std::function<void(luabridge::Namespace&)>& action) const;

    #pragma endregion

    #pragma region State Access

    void With_State(const std::function<void(lua_State*)>& actions) const;

    template<class T>
    T Get_Value_From_State(const std::function<T(lua_State*)>& actions) const
    {
        return actions(Get_State());
    }

    /**
     * Note: Calling this will terminate execution of the source CFunction
     * context, so no need to return after calling this.
     */
    void Raise_Error(const std::string& message) const;

    template<typename... Args>
    void Raise_Error_Format(const std::string& message, Args&&... args) const
    {
        Raise_Error(
            std::vformat(message, std::make_format_args(args...))
        );
    };

    #pragma endregion

    #pragma region Code Exec

    LuaResult Exec(const std::string& script) const;

    std::future<LuaResult> Exec_Async(const std::string& script) const;

    LuaResult Exec_File(const std::filesystem::path& script_path) const;

    LuaResult Exec_File_If_Exists(const std::filesystem::path& script_path) const;

    std::future<LuaResult> Exec_File_Async(const std::filesystem::path& script_path) const;

    LuaResult PCall(std::string_view expression) const;

    template<LuaPushType... Args>
    LuaResult PCall_With_Args(std::string_view expression, Args&&... args) const
    {
        if (!Is_Function()) {
            return {
                std::format(
                    "Unable to call '{}' as it is either undefined or not a function",
                    expression
                )
            };
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

    int Get_Stack_Count() const;

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
            } else {
                throw std::invalid_argument("Unsupported LuaVariant type - this is normally caused by variant type list being updated without updating supporting code");
            }
        });
    }

    int Get_Lua_Type_Code(int stack_index = -1) const;

    std::string_view Get_Lua_Type(const int& stack_index = -1) const;

    bool Is_Nil(int stack_index = -1) const;

    bool Is_None(int stack_index = -1) const;

    bool Is_Table(int stack_index = -1) const;

    bool Is_Function(int stack_index = -1) const;

    int Load_Global(std::string_view name) const;

    LuaResult With_Global(
        std::string_view name,
        const int& expected_lua_type,
        const std::function<LuaResult()>& action
    ) const;

    int Load_Table_Field(std::string_view name, int stack_index = -1) const;

    LuaResult With_Table_Field(
        std::string_view parent_expression,
        std::string_view name,
        const int& expected_lua_type,
        const std::function<LuaResult()>& action
    ) const;

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
            } else {
                throw std::invalid_argument("Unsupported LuaVariant type - this is normally caused by variant type list being updated without updating supporting code");
            }
        });
    }

    LuaResultWithValue<LuaVariant> Try_Read_Variant(const int& stack_index = -1) const;

    const std::string_view Get_Variant_Type(const LuaVariant& lua_variant) const;

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
        } else {
            throw std::invalid_argument("Unsupported LuaVariant type - this is normally caused by variant type list being updated without updating supporting code");
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

    LuaResultWithValue<std::string> To_String(int stack_index = -1) const;

    void Pop(const int& amount = 1) const;

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
                lua_pushnumber(L, static_cast<double>(value));
            } else if constexpr (
                std::is_same_v<T, int> ||
                std::is_same_v<T, uint> ||
                std::is_same_v<T, char> ||
                std::is_same_v<T, uchar>||
                std::is_same_v<T, ushort>
            ) {
                lua_pushinteger(L, static_cast<lua_Integer>(value));
            } else if constexpr (std::is_same_v<T, bool>) {
                lua_pushboolean(L, value);
            } else {
                throw std::invalid_argument("Unsupported LuaPushType type - this is normally caused by type list being updated without updating supporting code");
            }
        });
    }

    template<LuaPushType... Args>
    void Push_Values(Args&&... args) const
    {
        ((Push_Value(args)), ...);
    }

    void Push_Nil() const;

    bool Iterate_Over_Table(int stack_index = -1) const;

    template<LuaPushType T>
    LuaResult Set_Table_Field(
        std::string_view table_expression,
        std::string_view name,
        T value,
        const int& table_stack_index = -1
    ) const
    {
        if (!Is_Table(table_stack_index)) {
            return {
                std::format(
                    "Unable to set field '{}' as target '{}' is not a table",
                    name,
                    table_expression
                )  
            };
        }

        Push_Value(std::move(value));

        With_State([&](auto L) {
            lua_setfield(L, table_stack_index - 1, name.data());
        });

        return {LUA_OK};
    }

    #pragma endregion

    #pragma region Expressions

    template<LuaVariantCompatible T>
    LuaResultWithValue<T> Eval(const std::string& expression) const
    {
        if (const auto result = Exec(std::format("return {}", expression)); !result.Is_Ok()) {
            return LuaResultWithValue<T>(result);
        }

        return Try_Read<T>();
    }

    template<LuaVariantCompatible T>
    std::future<LuaResultWithValue<T>> Eval_Async(const std::string& expression) const
    {
        auto promise = std::make_shared<std::promise<LuaResultWithValue<T>>>();
        auto future = promise->get_future();

        std::thread([this, expression, promise]() {
            promise->set_value(
                Eval<T>(expression)
            );
        }).detach();

        return future;
    }

    #pragma endregion

    // for API building
    luabridge::Namespace Bridge() const;

    const std::vector<std::filesystem::path>& Get_Lua_Paths() const;

    virtual const std::string& Get_Id() const = 0;


protected:
    static inline const auto& Logger = CncLogger::For(LuaEngine);

    /**
     * Default paths for lua script files - we ensure this is in the Lua 'package.path'. See: UniqueLuaEngine()
     */
    std::vector<std::filesystem::path> LuaPaths;
    std::vector<std::string_view> RegisteredApis;

    void Init_Paths();

    virtual lua_State* Get_State() const = 0;
};

/**
 * Smart pointer helper class to teardown lua_State pointers.
 */
class LuaStateDeleter
{
public:
    void operator()(lua_State *L) const;

private:
    static inline const auto& Logger = CncLogger::For(LuaStateDeleter);
};

/**
 * Instances of this LuaEngine should be created for the lifecycle of a
 * given context to ensure clean state when starting a new context
 * (scenario/screen/thread etc.).
 * 
 * Aligns with smart pointer unique logic.
 */
class UniqueLuaEngine final : public LuaEngine
{
public:
    /**
     * Global lua state for the lifetime of the C++ process.
     *
     * Note: This state is NOT shared or accessible from instances of UniqueLuaEngine.
     */
    static const UniqueLuaEngine& Global();

    UniqueLuaEngine();

    const std::string& Get_Id() const override;
protected:
    lua_State* Get_State() const override;

private:
    std::unique_ptr<lua_State, LuaStateDeleter> State;
    std::string Id;

    static lua_State* Build_State();;
};

/**
 * Instances of this LuaEngine should be created to wrap around
 * a state that isn't owned by the current context, e.x. in a
 * Lua CFunction.
 * 
 * Aligns with smart pointer shared logic.
 */
class SharedLuaEngine final : public LuaEngine
{
public:
    SharedLuaEngine(lua_State* L);

    const std::string& Get_Id() const override;
protected:
    lua_State* Get_State() const override;

private:
    lua_State* State;
    std::string Id;
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
