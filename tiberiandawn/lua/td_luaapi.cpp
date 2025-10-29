#include "td_luaapi.h"

TiberianDawnLuaApi::TiberianDawnLuaApi(const std::string_view name, const bool has_native_module, const std::vector<std::filesystem::path> scripts): LuaApi(name, has_native_module)
{
    std::vector<std::filesystem::path> td_scripts;

    // Update all scripts to be in the @property{TiberianDawnLuaApi::Td_Directory}
    // subdirectory of the nco library.
    std::transform(
        scripts.begin(),
        scripts.end(),
        std::back_inserter(td_scripts),
        [](const auto& s) { return Td_Directory / s; }
    );

    Scripts = td_scripts;
}

const std::string& TiberianDawnLuaApi::Get_Parent_Lua_Module_Path() const
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
