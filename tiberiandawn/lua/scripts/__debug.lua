local Utils = require("nco.lib.Utils")

--[[
  This script is invoked by the debug key listener routine
  in Tiberian Dawn. Press F5 to execute the file; this is
  not cached so the file can be edited whilst the game is running.

  See: `Debug_Key()` in `tiberiandawn/debug.cpp`
]]
local function onF5()
  Logger.debug("onF5 Called")

  Logger.debug("Houses: %s", Utils.arrayToCsv(Scenario.houses.getNames()))

  local value = Rules["Game.Map"].MaxBuildDistance

  Messages.sendToPlayer(
    "HELLO THERE, Game.Map=>MaxBuildDistance type: %s",
    Rules["Game.Map"].getRuleType("MaxBuildDistance")
  )
  UI.showPopup("OK", "This message came from Lua. Game.Map=>MaxBuildDistance: %d", value)

  Rules["Game.Harvesting"].CreditsPerTiberiumScoop = 250

  for _, r in ipairs(Rules["Game.Misc"].getRuleNames()) do
    Logger.info("Section rule: [Game.Misc] => %s", r)
  end

  local goodguy = Scenario.houses.GoodGuy

  local money = goodguy.getMoney()

  Logger.info("GDI money: %s", money)

  -- should resolve to +100 money
  goodguy.giveMoney(200)
  goodguy.takeMoney(100)
end

onF5()
