- This script generates `.rockspec` file for use with LuaRocks, which contains the Lua packages found in [common/lua/scripts/nco](../../common/lua/scripts/nco) and [tiberiandawn/lua/scripts/nco](../../tiberiandawn/lua/scripts/nco)
- Jinja template: [cnc-nco.rockspec.j2](../../cnc-nco.rockspec.j2)
- You need a recent version of Python 3 to run the script

## Running

To manually run the script:

- Open a terminal in Linux/MacOS OR Git Bash/WSL on Windows
- Run `./generate.sh`
- Template will be rendered to [cnc-nco-scm-0.rockspec](../../cnc-nco-scm-0.rockspec)

To specify a version for the rock:

- Open a terminal in Linux/MacOS OR Git Bash/WSL on Windows
- Run `./generate.sh <version>-0`
- Template will be rendered to the root of the repo in `cnc-<version>-0.rockspec`

> Note: LuaRocks versions must always end with `-[0-9]`
