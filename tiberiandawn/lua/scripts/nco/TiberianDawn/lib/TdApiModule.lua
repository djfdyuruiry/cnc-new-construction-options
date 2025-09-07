local ApiModule = require("nco.lib.ApiModule")

--- Loads an native Lua API module, using a builder pattern and
--- the option to mock the backend C++ API using CncApiMock.
--- 
--- Returned table is a wrapper around the native Lua module locking
--- it down to be read only.
--- 
--- Note: This proxies nco.lib.ApiModule, adds extra Tiberian Dawn
---       setup before/after module build.
---@generic T : ApiModule
---@param moduleSpec ApiModuleSpec
---@return T
return function(moduleSpec)
  return ApiModule(moduleSpec)
end
