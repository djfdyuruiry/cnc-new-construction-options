_G.TiberianDawn = _G.TiberianDawn and _G.TiberianDawn or {}

-- See: tiberiandawn/lua/scenario_luaapi.h
_G.TiberianDawn.Scenario = _G.TiberianDawn.Scenario and _G.TiberianDawn.Scenario or {
  name = __CNC_API.Scenario.name,
  type = __CNC_API.Scenario.type,
  faction = __CNC_API.Scenario.faction,
  house = __CNC_API.Scenario.house,

  getTriggerNames = __CNC_API.Scenario.getTriggerNames,
  deleteTriggerIfExists = __CNC_API.Scenario.deleteTriggerIfExists
}
