local ApiModule = require("nco.lib.ApiModule")

--- This proxies nco.lib.ApiModule, adds extra Tiberian Dawn
--- setup before/after module build.
---@generic T : ApiModule
---@param moduleSpec ApiModuleSpec
---@return T
return function(moduleSpec)
  return ApiModule(moduleSpec)
end
