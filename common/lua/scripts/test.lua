local CncApiMock = require("nco.lib.CncApiMock")
local Logger = require("nco.Logger")

Logger.debug("Get better %s", "mate")

Logger.level

assert(
  #CncApiMock.__calls.Logger.log == 1,
  "Logger.debug didn't call C++ log function"
)
