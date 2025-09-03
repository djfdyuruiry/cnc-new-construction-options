local RulesApiProxy = require("nco.lib.RulesApiProxy")

-- See: common/lua/rules_luaapi.h
_G.Rules = RulesApiProxy(__CNC_API.Rules)
