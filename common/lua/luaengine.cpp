#include "../paths.h"

#include "luaengine.h"

const std::filesystem::path& LuaEngine::Get_Lua_Path()
{
    static const auto lua_path = std::filesystem::path(Paths.Program_Path()) / "lua";

    return lua_path;
}

luabridge::Namespace LuaEngine::Get_Api_Namespace(const std::string_view name) const
{
    return Bridge()
      .beginNamespace(RootApiNamespace.data())
      .beginNamespace(name.data());
}

void LuaEngine::With_Api_Namespace(const std::string_view name, const std::function<void(luabridge::Namespace&)>& action) const
{
    auto api_namespace = Get_Api_Namespace(name);

    action(api_namespace);

    api_namespace.endNamespace().endNamespace();
}

void LuaEngine::With_State(const std::function<void(lua_State*)>& actions) const
{
    actions(Get_State());
}

void LuaEngine::Raise_Error(const std::string& message) const
{
    With_State([&](auto L) {
        Push_Value(message);
        lua_error(L);
    });
}

LuaResult LuaEngine::Exec(const std::string& script) const
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

std::future<LuaResult> LuaEngine::Exec_Async(const std::string& script) const
{
    auto promise = std::make_shared<std::promise<LuaResult>>();
    auto future = promise->get_future();

    std::thread([this, script, promise]() {
        const auto result = Exec(script);

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

std::filesystem::path LuaEngine::Resolve_Script_Path(const std::filesystem::path& script_path) const
{
    auto full_script_path = script_path;

    if (script_path.is_relative()){
        // assume relative paths are part of @var{Lua_Path} file tree
        full_script_path = Get_Lua_Path() / script_path;
    }

    return full_script_path;
}

LuaResult LuaEngine::Exec_File(const std::filesystem::path& script_path) const
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

LuaResult LuaEngine::Exec_File_If_Exists(const std::filesystem::path& script_path) const
{
    const auto full_script_path = Resolve_Script_Path(script_path);

    if (!std::filesystem::is_regular_file(full_script_path)) {
        CNC_LOGGER_WARN("Skipping lua file execution as it does not exist: {}", full_script_path.string());
        return {LUA_OK};
    }

    return Exec_File(full_script_path);
}

std::future<LuaResult> LuaEngine::Exec_File_Async(const std::filesystem::path& script_path) const
{
    auto promise = std::make_shared<std::promise<LuaResult>>();
    auto future = promise->get_future();

    std::thread([this, script_path, promise]() {
        const auto result = Exec_File(script_path);

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

LuaResult LuaEngine::PCall(std::string_view expression) const
{
    if (!Is_Function()) {
        return{
            std::format(
                "Unable to call '{}' as it is either undefined or not a function",
                expression
            )
        };
    }

    return Get_Value_From_State<LuaResult>([](auto L) {
        return LuaResult(
            L,
            lua_pcall(L, 0, LUA_MULTRET, 0)
        );
    });
}

int LuaEngine::Get_Stack_Count() const
{
    return lua_gettop(Get_State());
}

int LuaEngine::Get_Lua_Type_Code(int stack_index) const
{
    return Get_Value_From_State<bool>([&](auto L) {
        return lua_type(L, stack_index);
    });
}

std::string_view LuaEngine::Get_Lua_Type(const int& stack_index) const
{
    const auto type_code = Get_Lua_Type_Code(stack_index);

    return LuaTypeMap[type_code].value();
}

bool LuaEngine::Is_Nil(int stack_index) const
{
    return Get_Value_From_State<bool>([&](auto L) {
        return lua_type(L, stack_index) == LUA_TNIL;
    });
}

bool LuaEngine::Is_None(int stack_index) const
{
    return Get_Value_From_State<bool>([&](auto L) {
        return lua_type(L, stack_index) == LUA_TNONE;
    });
}

bool LuaEngine::Is_Table(int stack_index) const
{
    return Get_Value_From_State<bool>([&](auto L) {
        return lua_istable(L, stack_index);
    });
}

bool LuaEngine::Is_Function(int stack_index) const
{
    return Get_Value_From_State<bool>([&](auto L) {
        return lua_isfunction(L, stack_index);
    });
}

int LuaEngine::Load_Global(std::string_view name) const
{
    return Get_Value_From_State<int>([&](auto L) {
        return lua_getglobal(L, name.data());
    });
}

LuaResult LuaEngine::With_Global(std::string_view name, const int& expected_lua_type, const std::function<LuaResult()>& action) const
{
    if (const auto type = Load_Global(name); type != expected_lua_type) {
        Pop();
        return {
            std::format(
                "Global '{}' was of unexpected type '{}', expected '{}'",
                name,
                LuaTypeMap[type].value(),
                LuaTypeMap[expected_lua_type].value()
            )
        };
    }

    auto result = action();

    Pop();

    return result;
}

int LuaEngine::Load_Table_Field(std::string_view name, int stack_index) const
{
    return Get_Value_From_State<int>([&](auto L) {
        return lua_getfield(L, stack_index, name.data());
    });
}

LuaResult LuaEngine::With_Table_Field(std::string_view parent_expression, std::string_view name, const int& expected_lua_type, const std::function<LuaResult()>& action) const
{
    const auto type = Load_Table_Field(name);

    if (type != expected_lua_type) {
        Pop();
        return {
            std::format(
                "Table field '{}.{}' was of unexpected type '{}', expected '{}'",
                parent_expression,
                name,
                LuaTypeMap[type].value(),
                LuaTypeMap[expected_lua_type].value()
            )
        };
    }

    const auto result = action();
    Pop();

    return result;
}

LuaResultWithValue<LuaVariant> LuaEngine::Try_Read_Variant(const int& stack_index) const
{
    return Get_Value_From_State<LuaResultWithValue<LuaVariant>>([&](auto L) {
        std::optional<LuaVariant> variant_value;

        if (Is_Type<int>(stack_index)) {
            variant_value = Try_Read<int>(stack_index).Unpack();
        } else if (Is_Type<double>(stack_index)) {
            variant_value = Try_Read<double>(stack_index).Unpack();
        } else if (Is_Type<bool>(stack_index)) {
            variant_value = Try_Read<bool>(stack_index).Unpack();
        } else if (Is_Type<std::string>(stack_index)) {
            variant_value = Try_Read<std::string>(stack_index).Unpack();
        } else {
            throw std::invalid_argument("Unsupported LuaVariant type - this is normally caused by variant type list being updated without updating supporting code");
        }

        if (variant_value.has_value()) {
            return LuaResultWithValue(variant_value.value());
        }

        return LuaResultWithValue<LuaVariant>(
            Get_State(),
            std::format("Attempted to read Lua value as variant, but type not supported by variant: {}", Get_Lua_Type(stack_index))
        );
    });
}

const std::string_view LuaEngine::Get_Variant_Type(const LuaVariant& lua_variant) const
{
    if (std::holds_alternative<std::string>(lua_variant)) {
        return LuaTypeMap[LUA_TSTRING].value();
    }
    if (std::holds_alternative<int>(lua_variant)) {
        return LuaTypeMap[LUA_TNUMBER].value();
    }
    if (std::holds_alternative<double>(lua_variant)) {
        return LuaTypeMap[LUA_TNUMBER].value();
    }
    if (std::holds_alternative<bool>(lua_variant)) {
        return LuaTypeMap[LUA_TBOOLEAN].value();
    }

    throw std::invalid_argument("Unsupported LuaVariant type - this is normally caused by variant type list being updated without updating supporting code");
}

LuaResultWithValue<std::string> LuaEngine::To_String(int stack_index) const
{
    return Get_Value_From_State<LuaResultWithValue<std::string>>([&](auto L) {
        return LuaResultWithValue<std::string>(
            std::string(lua_tostring(L, stack_index))
        );
    });
}

void LuaEngine::Pop(const int& amount) const
{
    With_State([&](auto L) {
        lua_pop(L, amount);
    });
}

void LuaEngine::Push_Nil() const
{
    With_State([](auto L){ lua_pushnil(L); });
}

bool LuaEngine::Iterate_Over_Table(int stack_index) const
{
    return Get_Value_From_State<bool>([&](auto L){ return lua_next(L, stack_index) != 0; });
}

void LuaStateDeleter::operator()(lua_State* L) const
{
    if (L)
    {
        lua_close(L);
    }
}

const UniqueLuaEngine& UniqueLuaEngine::Global()
{
    static UniqueLuaEngine global;

    return global;
}

UniqueLuaEngine::UniqueLuaEngine(): State(Build_State(), LuaStateDeleter())
{
    With_Global("package", LUA_TTABLE, [&]() {
        auto read_result = Try_Read_Table_Field<std::string>("package", "path");

        if (!read_result.Is_Ok() || !read_result.Has_Value()) {
            return static_cast<LuaResult>(read_result);
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

lua_State* UniqueLuaEngine::Get_State() const
{
    return State.get();
}

lua_State* UniqueLuaEngine::Build_State() {
    const auto L = luaL_newstate();
    luaL_openlibs(L);

    return L;
}

lua_State* SharedLuaEngine::Get_State() const
{
    return State;
}
