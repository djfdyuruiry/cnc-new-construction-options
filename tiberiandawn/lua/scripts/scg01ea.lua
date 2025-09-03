local function main()
  Logger.info("Someone called scg01ea.lua")

  for _, t in ipairs(TiberianDawn.Scenario.getTriggerNames()) do
    Logger.debug("Trigger: %s", t)
  end

  Logger.info("Preventing GDI getting Hum-Vee's")

  if not TiberianDawn.Scenario.deleteTriggerIfExists("RNF4") then
    Logger.warning("Attempted to delete missing trigger: RNF4")
  end

  if not TiberianDawn.Scenario.deleteTriggerIfExists("RNF6") then
    Logger.warning("Attempted to delete missing trigger: RNF6")
  end
end

main()
