local TdApiModule = require("nco.TiberianDawn.lib.TdApiModule")
local TypeApiProxy = require("nco.TiberianDawn.lib.TypeApiProxy")

--[[
  API that allows you to read/write properties of various types in
  the game. The property names and types are the same as the INI
  files for types. (Units, Buildings, Infantry etc.)

  Note: Changing a rule value will affect the game behavior immediately.

  Example:

  - Using the below infantry.ini section as a reference:
    ```ini
      ; Minigunner
      [E1]
      Armor=NONE
      Cost=100
      ; more rules...
      Prerequisite=NONE
      Primary=M16
      Reward=10
      ; more rules...
    ```
  - You can access this by:
    ```lua
      -- get a property
      local armor = Types.Infantry.E1.Armor -- 'NONE'

      -- set a property
      Types.Infantry.E1.Primary = "CHAIN_GUN"

      -- get the names of all properties for a type
      local infantryProperties = Types.Infantry.getPropertyNames() -- { 'Armor', 'Cost', ... }

      -- get the lua type for a given property
      local costType = Types.Infantry.E1.getPropertyType("Cost") -- 'number'

      -- get the names of all Infantry types in the game
      local infantryNames = Types.Infantry.getInstanceNames() -- { 'E1', 'E2', 'E2', ... }
    ```

]]
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
