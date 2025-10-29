#include "luaresult.h"

LuaResult::LuaResult(lua_State* L, int code): LuaCode(code) {
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
            DebugInfo = debug;
        }

        lua_pop(L, 1);
    }
}

LuaResult::LuaResult(std::string error): LuaCode(LUA_ERRERR + 1) {
    Error = std::make_optional(error);
}

bool LuaResult::Is_Ok() const{
    return LuaCode == LUA_OK;
}

std::string_view LuaResult::Code_As_String() const {
    return LuaErrorMap[LuaCode].value_or("Unknown");
}

std::string LuaResult::Error_Message() const {
    return Error.value_or("unknown error");
}

const LuaResult& LuaResult::If_Ok(const std::function<void(const LuaResult&)>& action) const {
    if (Is_Ok()) {
        action(*this);
    }

    return *this;
}

const LuaResult& LuaResult::On_Error(const std::function<void(const LuaResult&)>& action) const {
    if (!Is_Ok()) {
        action(*this);
    }

    return *this;
}

bool LuaResult::Has_Debug_Info() const {
    return DebugInfo.has_value();
}

const std::optional<lua_Debug>& LuaResult::Debug_Info() const {
    return DebugInfo;
}
