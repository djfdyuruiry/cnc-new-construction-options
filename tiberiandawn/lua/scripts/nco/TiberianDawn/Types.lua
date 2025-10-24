local TdApiModule = require("nco.TiberianDawn.lib.TdApiModule")
local TypeApiProxy = require("nco.TiberianDawn.lib.TypeApiProxy")

local function builder(cppApi)
  local api = {
    getTypeNames = cppApi.getTypeNames
  }

  for _, t in ipairs(cppApi.getTypeNames()) do
    api[t] = TypeApiProxy(cppApi, t)
  end

  return api
end

_G.Types = TdApiModule({
  name = "Types",
  cppSource = "tiberiandawn/lua/tdtypes_luaapi.h",
  builder = builder
})

return _G.Types
