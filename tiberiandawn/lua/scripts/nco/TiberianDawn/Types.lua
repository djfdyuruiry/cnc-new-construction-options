local TdApiModule = require("nco.TiberianDawn.lib.TdApiModule")

_G.Types = TdApiModule({
  name = "Types",
  cppSource = "tiberiandawn/lua/tdtypes_luaapi.h",
  builder = function(cppApi)
    return
    {
      getInfantryPropertyNames = cppApi.getInfantryPropertyNames,
      getInfantryPropertyValue = cppApi.getInfantryPropertyValue,
      setInfantryPropertyValue = cppApi.setInfantryPropertyValue
    }
  end
})

return _G.Types
