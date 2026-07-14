#include "td_luaapi.h"

#include <algorithm>

TiberianDawnLuaApi::TiberianDawnLuaApi(
    const std::string_view name,
    const bool has_native_module,
    const std::vector<std::filesystem::path>& scripts
): LuaApi(name, has_native_module)
{
    Scripts.clear();

    // Update all scripts to be in the @property{TiberianDawnLuaApi::Td_Directory}
    // subdirectory of the nco library.
    std::ranges::transform(
        scripts,
        std::back_inserter(Scripts),
        [](const auto& s) { return Td_Directory / s; }
    );
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
