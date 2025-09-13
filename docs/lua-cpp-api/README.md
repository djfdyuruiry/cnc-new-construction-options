# Lua/C++ API Documentation

This directory contains comprehensive documentation for the Lua/C++ integration system used in this project.

## Documentation Structure

- **index.md** - Main overview and architecture
- **engine.md** - Detailed documentation for LuaEngine class
- **unique_engine.md** - Documentation for UniqueLuaEngine 
- **api.md** - Detailed documentation for LuaApi
- **builder.md** - Information about LuaEngineBuilder
- **result.md** - Details on LuaResult classes
- **arguments.md** - Details on LuaArguments class
- **summary.md** - Complete reference guide

## Getting Started

To use the API:
1. Include necessary headers (`luaengine.h`, `luaresult.h`)
2. Create or obtain a Lua engine instance (UniqueLuaEngine, SharedLuaEngine)
3. Register APIs using LuaApi classes
4. Execute Lua scripts or functions

## Key Components

- **LuaEngine** - Base interface for all interactions
- **UniqueLuaEngine** - Context-specific engines with clean state
- **SharedLuaEngine** - For shared contexts
- **LuaResult** - Error handling and value extraction

## Usage Examples

See individual documentation files for code examples.
