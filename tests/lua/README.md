# Test Lua Scripts

- Run `./test-game-script.lua` to execute any game script on demand:
  - This uses a mock C++ API backend
  - Script paths are relative to the root of the repository
  - Example: `./test-game-script.lua tiberiandawn/lua/scripts/scg01ea.lua`
- Unit tests are stored in `tests/unit`
  - Execute `./tests/unit/init.lua`
