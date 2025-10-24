local TdApiModule = require("nco.TiberianDawn.lib.TdApiModule")
local TypeApiProxy = require("nco.TiberianDawn.lib.TypeApiProxy")

---@class Types : ApiModule
---@field getTypeNames fun(): string[]

---@return Types
local function builder(cppApi)
  return setmetatable(
    {
      getTypeNames = cppApi.getTypeNames
    },
    {
      __index = function(_, typeName)
        return TypeApiProxy(cppApi, typeName)
      end
    }
  )
end

---@type Types | { [string]: TypeApiProxy }
_G.Types = TdApiModule({
  name = "Types",
  cppSource = "tiberiandawn/lua/tdtypes_luaapi.h",
  builder = builder
})

return _G.Types
