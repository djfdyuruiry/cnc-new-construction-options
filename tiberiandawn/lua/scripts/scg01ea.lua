-- house goodguy scenario 1-east load test script
local Utils = require("nco.lib.Utils")

local function main()
  Logger.info("Someone called scg01ea.lua")

  Logger.debug("Triggers: %s", Utils.arrayToCsv(Scenario.triggers.getNames()))
  Logger.info("Preventing GDI getting Hum-Vee's")

  if not Scenario.triggers.deleteIfExists("RNF4") then
    Logger.warning("Attempted to delete missing trigger: RNF4")
  end

  if not Scenario.triggers.deleteIfExists("RNF6") then
    Logger.warning("Attempted to delete missing trigger: RNF6")
  end

  -- test trigger lookup
  Logger.debug("WIN trigger: %s", Scenario.triggers.get("WIN"))

  -- test lua trigger defined in scenario ini
  Event.handlers.onTimerTrigger = function(triggerName)
    Logger.debug("Handling trigger %s", triggerName)

    if triggerName == "TMR1" then
      Logger.debug("5 second trigger execute")
    elseif triggerName == "TMR2" then
      Logger.debug("10 second trigger execute")
    end
  end

  -- test team type lookups
  Logger.debug("Team Types: %s", Utils.arrayToCsv(Scenario.teams.getNames()))
  Logger.debug("Team GDIR2: %s", Scenario.teams.get("GDIR2"))

  -- test adding a new trigger
  Event.handlers.newTriggerTest = function(triggerName)
      Logger.debug("10 second trigger execute (%s)", triggerName)
  end

  Scenario.triggers.add("TMR3", "Time,Lua Event,15,GoodGuy,newTriggerTest,0")

  -- test adding a team
  Scenario.teams.add("LUA1", "GoodGuy,0,0,0,0,0,7,3,0,0,2,MTNK:1,LST:1,0,1,1")
  Scenario.triggers.add("TMR4", "Time,Reinforce.,20,GoodGuy,LUA1,0")
end

main()
