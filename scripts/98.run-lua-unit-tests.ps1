$ErrorActionPreference = "Stop"

$libScriptsPath = Split-Path $PSScriptRoot -Parent

$repoPath = Resolve-Path "$PSScriptRoot/.."

$scriptsPath = Join-Path $repoPath "scripts"
$scriptsBinPath = Join-Path $scriptsPath "bin"

$testsLuaScriptsPath = Join-Path $repoPath "tests/lua/scripts"

$env:EXTRA_LUA_PATH = "$testsLuaScriptsPath/?.lua;$testsLuaScriptsPath/?/init.lua"

& "$scriptsBinPath/lua.ps1" "$testsLuaScriptsPath/tests/unit/init.lua"
