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

  Event.handlers.onTimerTrigger = function(triggerName)
    Logger.debug("Handling trigger %s", triggerName)

    if triggerName == "TMR1" then
      Logger.debug("5 second trigger execute")
    elseif triggerName == "TMR2" then
      Logger.debug("10 second trigger execute")
    end
  end
end

main()
