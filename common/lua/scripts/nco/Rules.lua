if type(__CNC_API) == "nil" or (__CNC_API.Rules) == "nil" then
  error("nco.Rules failed to init, required C++ backend not loaded: common/lua/rules_luaapi.h")
end

local RulesApiProxy = require("nco.lib.RulesApiProxy")

_G.Rules = RulesApiProxy(__CNC_API.Rules)

return _G.Rules
