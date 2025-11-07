# API Reference (Generic)

This document describes the generic patterns and concepts for using the Lua native APIs, without specific implementation details.

## Overview

The Lua native library exposes C++ game engine functionality through a set of APIs that can be accessed from Lua scripts. Each API module provides access to different aspects of the game engine.

## Available APIs

### Rules API (`Rules`)
Provides access to game rules and configuration settings.
- Allows reading and writing rule values
- Supports accessing sections and rule keys from rules.ini
- Enables dynamic modification of game behavior
- Uses the `RuleSectionsProviderConcept` interface to allow providing a game engine type that resolves a `RuleSections` instance to read/write rules from (*it is expected a static class is implemented that matches this inside a game engine*)

```c++
template <typename T>  
concept RuleSectionsProviderConcept = requires()  
{  
    { T::Sections() } -> std::same_as<RuleSections&>;  
};
```

### System API (`System`)
Provides access to system functionality and file operations.
- Path handling utilities
- File opening functions for game paths
- OS information access
- Path manipulation capabilities

## Generic Usage Pattern

All API modules follow the same usage pattern:

```lua
-- Access the API (typically global variables)
local rules = Rules        -- or
local system = System      -- etc.

-- Use the API
local value = rules["Section"].RuleName
rules["Section"].RuleName = newValue
```

## Module Structure

Each API module:
1. Is loaded using `ApiModule()` with a specification
2. Has a builder function that creates its interface
3. Provides a read-only access pattern to prevent modification
4. Supports mocking for testing purposes

## Error Handling

API calls follow these patterns:
- Validation occurs at load time
- Runtime errors are propagated with descriptive messages
- Type checking ensures correct parameter usage

## Mocking Support

All APIs support the mocking system, allowing:
- Testing without running the game engine
- Call history recording for assertions
- Parameter capture for verification
