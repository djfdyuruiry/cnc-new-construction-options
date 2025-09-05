#pragma once

#include <filesystem>
#include <functional>
#include <string>

#include "../common/lua/luaapi.h"

class TiberianDawnLuaApi : public LuaApi
{
public:
    inline static const std::filesystem::path Td_Directory = "TiberianDawn";

    TiberianDawnLuaApi(const std::string_view name)
        : LuaApi(name)
    {
    }

    TiberianDawnLuaApi(const std::string_view name, const bool has_native_module)
        : LuaApi(name, has_native_module)
    {
    }

    TiberianDawnLuaApi(const std::string_view name, const bool has_native_module, const std::vector<std::filesystem::path> scripts)
        : LuaApi(name, has_native_module)
    {
        std::vector<std::filesystem::path> td_scripts;

        // Update all scripts to be in the @property{TiberianDawnLuaApi::Td_Directory} 
        // sub directory of the nco library.
        std::transform(
            scripts.begin(),
            scripts.end(),
            std::back_inserter(td_scripts),
            [](const auto& s) { return Td_Directory / s; }
        );

        Scripts = td_scripts;
    }

protected:
    virtual const std::string& Get_Parent_Lua_Module_Path() const override
    {
        static const std::string module(
            std::format(
                "{}.{}",
                LuaApi::Get_Parent_Lua_Module_Path(),
                Td_Directory.string()
            )
        );

        return module;
    }

};
