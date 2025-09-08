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
end

main()
