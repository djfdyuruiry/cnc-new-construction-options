if type(__CNC_API) == "nil" or (__CNC_API.Scenario) == "nil" then
  error("nco.TiberianDawn.Scenario failed to init, required C++ backend not loaded: -- tiberiandawn/lua/scenario_luaapi.h")
end

_G.TiberianDawn = _G.TiberianDawn and _G.TiberianDawn or {}

_G.TiberianDawn.Scenario = _G.TiberianDawn.Scenario and _G.TiberianDawn.Scenario or {
  __cpp_source = __CNC_API.Scenario.__cpp_source,
  __name = __CNC_API.Scenario.__name,

  name = __CNC_API.Scenario.name,
  type = __CNC_API.Scenario.type,
  faction = __CNC_API.Scenario.faction,
  house = __CNC_API.Scenario.house,

  getTriggerNames = __CNC_API.Scenario.getTriggerNames,
  deleteTriggerIfExists = __CNC_API.Scenario.deleteTriggerIfExists
}

return _G.TiberianDawn.Scenario
