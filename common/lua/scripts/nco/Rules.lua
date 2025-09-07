local ApiModule = require("nco.lib.ApiModule")
local RulesApiProxy = require("nco.lib.RulesApiProxy")

--[[
  API to read/write game engine rules, see rules.ini for
  names of sections and rule keys.

  Note: Changing a rule value will affect the game behavior immediately.

  Example:

  - For the rules.ini section:
    ```ini
      [Game.Map]
      MaxBuildDistance=1
      PreventBuildingInShroud=yes
    ```
  - You can access this by:
    ```lua
      -- get a rule value
      auto buildDistance = Rules["Game.Map"].MaxBuildDistance

      -- set a rule value
      Rules["Game.Map"].PreventBuildingInShroud = false

      -- you can capture the old value when setting it
      local oldValue = Rules["Game.Map"].PreventBuildingInShroud = false
    ```

  See: nco.lib.Path
]]
---@type RulesApiProxy
_G.Rules = ApiModule({
  name = "Rules",
  cppSource = "common/lua/rules_luaapi.h",
  builder = RulesApiProxy
})

return Rules
