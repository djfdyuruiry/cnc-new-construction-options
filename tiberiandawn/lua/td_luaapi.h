#pragma once

#include <filesystem>
#include <functional>
#include <string>

#include "../common/lua/luaapi.h"

class TiberianDawnLuaApi : public LuaApi
{
public:
    inline static const std::filesystem::path Td_Directory = "TiberianDawn";

    TiberianDawnLuaApi(const std::string_view name, const std::vector<std::filesystem::path> scripts)
        : LuaApi(name)
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

    TiberianDawnLuaApi(const std::string_view name)
        : LuaApi(name)
    {
    }
};
