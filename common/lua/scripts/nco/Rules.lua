local ApiModule = require("nco.lib.ApiModule")
local RulesApiProxy = require("nco.lib.RulesApiProxy")

return ApiModule({
  modulePath = {"Rules"},
  cppApi = "Rules",
  cppSource = "common/lua/rules_luaapi.h",
  builder = RulesApiProxy
})
