#include "paths.h"

#include "luaengine.h"

const TwoWayMap<int, std::string_view> LuaEngine::LuaTypeMap {
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

std::filesystem::path LuaEngine::Resolve_Script_Path(const std::filesystem::path& script_path) const
{
    if (!script_path.is_relative()) {
        return script_path;
    }

    // assume relative paths are part of @var{Lua_Path} file tree
    for (const auto& lua_path : LuaPaths) {
        const auto potential_path = lua_path / script_path;

        CNC_LOGGER_DEBUG("Checking for Lua script '{}' at path: {}", script_path.string(), potential_path.string());

        if (std::filesystem::exists(potential_path) && !std::filesystem::is_directory(potential_path)) {
            CNC_LOGGER_DEBUG("Found Lua script '{}' at path: {}", script_path.string(), potential_path.string());
            return potential_path;
        }
    }

    const auto default_lua_path = LuaPaths.at(0);
    const auto default_path = default_lua_path / script_path;

    CNC_LOGGER_WARN(
        "Unable to resolve lua script '{}' to any known path, defaulting to path: {}",
        script_path.string(),
        default_lua_path.string()
    );

    return default_path;
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

        // TODO: Idea - fire a popup/show message event, have some hook to report errors for game engine to extend,
        //       user would get a nice popup/message with the lua error (we could have a lua debug rule to control this)
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

LuaResult LuaEngine::Exec_File(const std::filesystem::path& script_path) const
{
    auto full_script_path = Resolve_Script_Path(script_path);

    CNC_LOGGER_DEBUG("Attempting to execute lua file: {}", full_script_path.string());

    return Get_Value_From_State<LuaResult>([&full_script_path](auto L) {
        auto status = luaL_loadfile(L, full_script_path.string().c_str());

        if (status != LUA_OK) {
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

LuaResult LuaEngine::Exec_File_If_Exists(const std::filesystem::path& script_path, bool& file_was_found) const
{
    const auto full_script_path = Resolve_Script_Path(script_path);

    file_was_found = std::filesystem::exists(full_script_path) && !std::filesystem::is_directory(full_script_path);

    if (!file_was_found) {
        CNC_LOGGER_WARN("Skipping lua file execution as it does not exist: {}", full_script_path.string());

        return {LUA_OK};
    }

    return Exec_File(full_script_path);
}

LuaResult LuaEngine::Exec_File_If_Exists(const std::filesystem::path& script_path) const
{
    bool temp;

    return Exec_File_If_Exists(script_path, temp);
}

std::future<LuaResult> LuaEngine::Exec_File_Async(const std::filesystem::path& script_path) const
{
    auto promise = std::make_shared<std::promise<LuaResult>>();
    auto future = promise->get_future();

    std::thread([this, script_path, promise]() {
        const auto result = Exec_File(script_path);

        // TODO: Idea - fire a popup/show message event, have some hook to report errors for game engine to extend,
        //       user would get a nice popup/message with the lua error (we could have a lua debug rule to control this)
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

const std::string_view& LuaEngine::Get_Lua_Type(const int& stack_index) const
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

const std::string_view& LuaEngine::Get_Variant_Type(const LuaVariant& lua_variant) const
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
        return LuaResultWithValue(
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

LuaEvalResult LuaEngine::Eval(const std::string& expression) const
{
    CNC_LOGGER_TRACE("Attempting to evaluate lua expression: {}", expression);

    return Get_Value_From_State<LuaEvalResult>([&expression](auto L)
    {
        // attempt to compile an evaluation first
        const auto return_expression = std::format("return {}", expression);

        auto status = luaL_loadstring(L, return_expression.c_str());
        const auto eval_returns_value = status == LUA_OK;

        if (!eval_returns_value) {
            // expression cannot be evaluated, just compile for execution instead
            status = luaL_loadstring(L, expression.c_str());
        }

        if (status != LUA_OK) {
            auto result = LuaResult(L, status);

            CNC_LOGGER_TRACE(
                "Error loading lua script due to '{}' error: {}",
                result.Code_As_String(),
                result.Error_Message()
            );
            return LuaEvalResult(result, eval_returns_value);
        }

        const auto eval_result = LuaResult(
            L,
            lua_pcall(L, 0, LUA_MULTRET, 0)
        );

        return LuaEvalResult(eval_result, eval_returns_value);
    });
}

LuaResultWithValue<std::string> LuaEngine::Eval_To_String(const std::string& expression) const
{
    CNC_LOGGER_TRACE("Attempting to evaluate lua expression as string: {}", expression);

    auto eval_result = Get_Value_From_State<LuaEvalResult>([&expression](auto L)
    {
        // attempt to compile an evaluation first
        const auto return_expression = std::format(
            "local ___r = {}; return tostring(___r == nil and 'nil' or ___r)", // tostring errors with nil arg
            expression
        );

        auto status = luaL_loadstring(L, return_expression.c_str());
        const auto eval_returns_value = status == LUA_OK;

        if (!eval_returns_value) {
            // expression cannot be evaluated, just compile for execution instead
            status = luaL_loadstring(L, expression.c_str());
        }

        if (status != LUA_OK) {
            auto result = LuaResult(L, status);

            CNC_LOGGER_TRACE(
                "Error loading lua script due to '{}' error: {}",
                result.Code_As_String(),
                result.Error_Message()
            );
            return LuaEvalResult(result, eval_returns_value);
        }

        const auto eval_result = LuaResult(
            L,
            lua_pcall(L, 0, LUA_MULTRET, 0)
        );

        return LuaEvalResult(eval_result, eval_returns_value);
    });

    if (!eval_result.Is_Ok() || !eval_result.Returned_Value()) {
        return { eval_result };
    }

    return To_String();
}

luabridge::Namespace LuaEngine::Bridge() const
{
    return luabridge::getGlobalNamespace(Get_State());
}

const std::vector<std::filesystem::path>& LuaEngine::Get_Lua_Paths() const
{
    return LuaPaths;
}

void LuaEngine::Init_Paths()
{
    LuaPaths.clear();
    LuaPaths.emplace_back(Paths.Program_Lua_Path());
    LuaPaths.emplace_back(Paths.User_Lua_Path());
}

void LuaStateDeleter::operator()(lua_State* L) const
{
    if (L)
    {
        // BUG: If Lua enabled for scenario editor, this log statement crashes formatter (Lua is disabled in editor)
        CNC_LOGGER_INFO("Destroying Lua state: {}", static_cast<void*>(L));
        lua_close(L);
    }
}

const UniqueLuaEngine& UniqueLuaEngine::Global()
{
    static UniqueLuaEngine global;

    return global;
}

UniqueLuaEngine::UniqueLuaEngine() :
    State(Build_State(), LuaStateDeleter()),
    Id(std::format("{}", static_cast<void*>(Get_State())))
{
    Init_Paths();
    With_Global("package", LUA_TTABLE, [&]() {
        auto read_result = Try_Read_Table_Field<std::string>("package", "path");

        if (!read_result.Is_Ok() || !read_result.Has_Value()) {
            return static_cast<LuaResult>(read_result);
        }

        return read_result.Map<LuaResult>([&](const auto& base_package_path) {
            std::stringstream package_path;

            package_path << base_package_path << ';';

            for (const auto& lua_path : LuaPaths) {
                package_path << std::format(
                    "{}{}?.lua;{}{}?{}init.lua",
                    lua_path.string(),
                    PathsClass::SEP,
                    lua_path.string(),
                    PathsClass::SEP,
                    PathsClass::SEP
                ) << ';';
            }

            const auto resolved_lua_path = package_path.str();

            CNC_LOGGER_INFO("Setting Lua package.path to: {}", resolved_lua_path);

            return Set_Table_Field("package", "path", resolved_lua_path);
        });
    }).On_Error([&](auto& r) {
        CNC_LOGGER_FATAL(
            "Failed to initialise Lua package paths: {} | Engine ID = {}",
            r.Error_Message(),
            Get_Id()
        );
    });
}

lua_State* UniqueLuaEngine::Get_State() const
{
    return State.get();
}

lua_State* UniqueLuaEngine::Build_State()
{
    const auto L = luaL_newstate();
    CNC_LOGGER_INFO("Created Lua state: {}", static_cast<void*>(L));

    CNC_LOGGER_DEBUG("Opening Lua libraries for state:", static_cast<void*>(L));
    luaL_openlibs(L);

    return L;
}

const std::string& UniqueLuaEngine::Get_Id() const
{
    return Id;
}

SharedLuaEngine::SharedLuaEngine(lua_State* L) : State(L), Id(std::format("{}", static_cast<void*>(L)))
{
    Init_Paths();
}

lua_State* SharedLuaEngine::Get_State() const
{
    return State;
}

const std::string& SharedLuaEngine::Get_Id() const
{
    return Id;
}
