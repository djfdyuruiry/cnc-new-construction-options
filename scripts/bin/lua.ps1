$ErrorActionPreference = "Stop"

$libScriptsPath = Split-Path $PSScriptRoot -Parent
$repoPath = Resolve-Path "$PSScriptRoot/../.."

$commonLuaScriptsPath = Join-Path $repoPath "common/lua/scripts"
$tdLuaScriptsPath = Join-Path $repoPath "tiberiandawn/lua/scripts"
$testLuaScriptsPath = Join-Path $repoPath "tests/lua/scripts"

# Check if lua command exists
if (-not (Get-Command -Name "lua" -ErrorAction SilentlyContinue)) {
    Write-Error "The Lua binary was not found on this PATH, this is required to run this wrapper"
    exit 1
}

# Get the default Lua package path
$luaPath = lua -e "print(package.path)"

# Add EXTRA_LUA_PATH if it exists
if ($env:EXTRA_LUA_PATH) {
    $luaPath = "$luaPath;$env:EXTRA_LUA_PATH"
}

# Add common and TD Lua script paths
$luaPath = "$luaPath;$commonLuaScriptsPath/?.lua;$commonLuaScriptsPath/?/init.lua"
$luaPath = "$luaPath;$tdLuaScriptsPath/?.lua;$tdLuaScriptsPath/?/init.lua"

# Set LUA_PATH environment variable
$env:LUA_PATH = $luaPath

# Execute Lua with all provided arguments
lua @args
