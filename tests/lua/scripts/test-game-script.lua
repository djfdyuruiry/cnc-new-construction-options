require("nco.TiberianDawn.lib.TdCncApiMock")
require("nco")
require("nco.TiberianDawn")

if type(arg[1]) ~= "string" then
  print(string.format("Usage: %s <game-script-path>", arg[0]))
  os.exit(1)
end

print(string.format("test-game-script.lua | Running game Lua script: %s", arg[1]))

dofile(arg[1])
