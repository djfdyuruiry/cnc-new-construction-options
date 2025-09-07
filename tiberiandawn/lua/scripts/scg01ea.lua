local function main()
  Logger.info("Someone called scg01ea.lua")

  local triggerCsv = ""

  for i, t in ipairs(Scenario.triggers.getNames()) do
    if i ~= 1 then
      triggerCsv = triggerCsv .. ","
    end

    triggerCsv = triggerCsv .. t
  end

  Logger.debug("Triggers: %s", triggerCsv)
  Logger.info("Preventing GDI getting Hum-Vee's")

  if not Scenario.triggers.deleteIfExists("RNF4") then
    Logger.warning("Attempted to delete missing trigger: RNF4")
  end

  if not Scenario.triggers.deleteIfExists("RNF6") then
    Logger.warning("Attempted to delete missing trigger: RNF6")
  end
end

main()
