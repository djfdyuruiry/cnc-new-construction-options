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
      local buildDistance = Rules["Game.Map"].MaxBuildDistance -- 1

      -- set a rule value
      Rules["Game.Map"].PreventBuildingInShroud = false

      -- get the lua type for a given rule
      local ruleType = Rules["Game.Map"].getRuleType("PreventBuildingInShroud") -- 'boolean'

      -- get a list of rules in a section
      local ruleType = Rules["Game.Map"].getRuleNames() -- { 'MaxBuildDistance', 'PreventBuildingInShroud', ... }

      -- get the names of all sections in rules.ini
      local sectionNames = Rules.getSectionNames() -- { 'Enhancements', ... }
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
