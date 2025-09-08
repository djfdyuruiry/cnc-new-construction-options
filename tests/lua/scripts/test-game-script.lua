require("nco.TiberianDawn.lib.TdCncApiMock")
require("nco")
require("nco.TiberianDawn")

if type(arg[1]) == "string" then
  print(string.format("test-game-script.lua | Running game Lua script: %s", arg[1]))

  dofile(arg[1])
else
  error("test-game-script.lua | No arguments provided")
end
