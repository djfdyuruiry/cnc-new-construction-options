- This script generates the markdown file [wiki/3c.Lua-API-Reference.md](../../wiki/wiki/3c.Lua-API-Reference.md.j2) from the inline doc comments inside Lua packages [common/lua/scripts/nco](../../common/lua/scripts/nco) and [tiberiandawn/lua/scripts/nco](../../tiberiandawn/lua/scripts/nco)
- Jinja template [wiki/2b.Tiberian-Dawn-Rules.md.j2](../../wiki/2b.Tiberian-Dawn-Rules.md.j2).
- It is executed by the deploy wiki github workflow: [.github/workflows/wiki.yml](../../.github/workflows/wiki.yml)
- You need a recent version of Python 3 to run the script
- You need to install the [Lua Language Server](https://marketplace.visualstudio.com/items?itemName=sumneko.lua) in VSCode
  - See [Manual Lua Language Server](#manual-lua-language-server) for an alternative if you don't want to install it

## Running

To manually run the script:

- Open a terminal in Linux/MacOS (On Windows: this script only works when using a WSL VSCode workspace - i.e. Linux tools in WSL)
- Run `./generate.sh`
- Template will be rendered to [wiki/3c.Lua-API-Reference.md](../../wiki/wiki/3c.Lua-API-Reference.md.j2)

## Manual Lua Language Server

> Note: This can be used to run the script without VSCode, or on Windows without WSL

To specify use the Lua Language Server manually:

- Go to https://github.com/LuaLS/lua-language-server/releases/latest
- Grab the latest package for your system and uncompress it somewhere
- Run `./generate.sh <path-to-language-server>` - replace `<path-to-language-server>` with the full path you used in the last step
- Template will be rendered to [wiki/3c.Lua-API-Reference.md](../../wiki/wiki/3c.Lua-API-Reference.md.j2)
