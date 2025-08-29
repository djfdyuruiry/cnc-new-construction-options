- This script generates the markdown file [wiki/2b.Tiberian-Dawn-Rules.md](../../wiki/2b.Tiberian-Dawn-Rules.md) from the rule JSON files in [tiberiandawn/rules](../../tiberiandawn/rules/) and Jinja template [wiki/2b.Tiberian-Dawn-Rules.md.j2](../../wiki/2b.Tiberian-Dawn-Rules.md.j2).
- It is executed by the deploy wiki github workflow: [.github/workflows/wiki.yml](../../.github/workflows/wiki.yml)
- You need a recent version of Python 3 to run the script

## Running

To manually run the script:

- Open a terminal in Linux/MacOS OR Git Bash/WSL on Windows
- Run `./generate.sh`
- Template will be rendered to [wiki/2b.Tiberian-Dawn-Rules.md](../../wiki/2b.Tiberian-Dawn-Rules.md)
