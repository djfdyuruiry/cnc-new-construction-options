#pragma once

#include <filesystem>
#include <string>

#include "common/lua/luaapi.h"

class TiberianDawnLuaApi : public LuaApi
{
public:
    static inline const std::filesystem::path Td_Directory = "TiberianDawn";

    TiberianDawnLuaApi(const std::string_view name) : LuaApi(name) {}

    TiberianDawnLuaApi(const std::string_view name, const bool has_native_module) : LuaApi(name, has_native_module) {}

    TiberianDawnLuaApi(std::string_view name, bool has_native_module, const std::vector<std::filesystem::path>& scripts);

protected:
    const std::string& Get_Parent_Lua_Module_Path() const override;
};
