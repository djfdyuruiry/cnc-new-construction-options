#include "paths.h"

#include "system_luaapi.h"

void SystemLuaApi::Register_Consts(LuaEngine& engine) const
{
    With_Api_Namespace(engine, [](auto& n) {
        n.addConstant("gamePath", Paths.Program_Path())
         .addConstant("userPath", Paths.User_Path())
         .addConstant("pathSeparator", PathsClass::SEP)
         .addConstant("isRemasteredMod", IsRemasteredMod)
         .addConstant("isMacOS", IsMacOS)
         .addConstant("isUnix", IsUnix)
         .addConstant("isWindows", IsWindows);
    });
}
