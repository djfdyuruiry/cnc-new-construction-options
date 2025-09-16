-- house goodguy scenario 1-east load test script
local Utils = require("nco.lib.Utils")

local function editRules()
  Rules["Game.Misc"].McvRedeployable = true
end

local function addingNewTeam()
  Scenario.teams.LUA1 = "GoodGuy,0,0,0,0,0,7,3,0,0,2,HTNK:1,LST:1,0,1,1"
  Scenario.triggers.TMR4 = "Time,Reinforce.,8,GoodGuy,LUA1,0"
end

local function addingNewTrigger()
  Event.handlers.newTriggerTest = function(triggerName)
      Logger.debug("10 second trigger execute (%s)", triggerName)
  end

  Scenario.triggers.TMR3 = "Time,Lua Event,15,GoodGuy,newTriggerTest,0"
end

local function teamTypeLookups()
  Logger.debug("Team Types: %s", Utils.arrayToCsv(Scenario.teams.getNames()))
  Logger.debug("Team GDIR2: %s", Scenario.teams.GDIR2)
end

local function timerTriggerSetup()
  Event.handlers.onTimerTrigger = function(triggerName)
    Logger.debug("Handling trigger %s", triggerName)

    if triggerName == "TMR1" then
      Logger.debug("5 second trigger execute")
    elseif triggerName == "TMR2" then
      Logger.debug("10 second trigger execute")
    end
  end
end

local function editingTriggers()
  Logger.info("Preventing GDI getting Hum-Vee's")

  if not Scenario.triggers.deleteIfExists("RNF4") then
    Logger.warning("Attempted to delete missing trigger: RNF4")
  end

  if not Scenario.triggers.deleteIfExists("RNF6") then
    Logger.warning("Attempted to delete missing trigger: RNF6")
  end
end

local function triggerLookups()
  Logger.debug("Triggers: %s", Utils.arrayToCsv(Scenario.triggers.getNames()))
  Logger.debug("WIN trigger: %s", Scenario.triggers.WIN)
end

local function main()
  Logger.info("Someone called scg01ea.lua")

  triggerLookups()
  editingTriggers()
  timerTriggerSetup()
  teamTypeLookups()
  addingNewTrigger()
  addingNewTeam()
  editRules()
end

main()
