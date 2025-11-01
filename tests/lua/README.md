# Test Lua Scripts

- Unit tests are stored in `tests/unit`
  - Execute `scripts/98.run-lua-unit-tests.sh` from the root of the repo to execute `scripts/tests/unit/init.lua`
- Integration tests are stored in `tests/integration` (WIP)
  - No way to execute these yet, they would need to loaded with the real C++ APIs at runtime
- Run `scripts/99.test-lua-script.sh` from the root of the repo to execute any game script on demand:
  - This uses a mock C++ API backend
  - It invokes `scripts/test-game-script.lua`
  - Example: `./test-game-script.lua tiberiandawn/lua/scripts/scg01ea.lua`

> `.ps1` versions of the above scripts exist to allow execution using Powershell Core (useful for Windows)
