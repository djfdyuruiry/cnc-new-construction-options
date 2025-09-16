# Lua/C++ API Documentation

This directory contains comprehensive documentation for the Lua/C++ integration system used in this project.

## Documentation Structure

- [Overview](1.overview.md) - Main overview and architecture
- [Engine](2.engine.md) - Detailed documentation for LuaEngine class
- [Unique Engine](3.unique_engine.md) - Documentation for UniqueLuaEngine 
- [Api](4.api.md) - Detailed documentation for LuaApi
- [Builder](5.builder.md) - Information about LuaEngineBuilder
- [Result](6.result.md) - Details on LuaResult classes
- [Arguments](7.arguments.md) - Details on LuaArguments class
- [Events](8.events.md) - Details on LuaEvent class
- [Game Engine Integration](9.game-engine-integration.md) - Specific details on how the game engines integrate with Lua
- [Reference Guide](10.reference-guide.md) - Complete reference guide

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
- **LuaApi** - Abstraction for registering C++ types, values and functions accessible from Lua
- **LuaArguments** - Fluent interface for validating args passed into C++ from Lua and reading their values

## Usage Examples

See individual documentation files for code examples.
