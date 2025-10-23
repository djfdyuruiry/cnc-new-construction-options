local TdApiModule = require("nco.TiberianDawn.lib.TdApiModule")

local function builder(cppApi)

  return setmetatable(
    {
      getTypes = cppApi.getTypes
    },
    {
      __index = function (_, typeName)
        return cppApi.getTeamType(teamName)
      end
    }
  )
end

_G.Types = TdApiModule({
  name = "Types",
  cppSource = "tiberiandawn/lua/tdtypes_luaapi.h",
  builder = builder
})

return _G.Types
