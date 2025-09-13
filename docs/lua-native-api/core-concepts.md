# Core Concepts

This document covers the fundamental concepts and patterns used in the Lua native library architecture.

## ApiModule System

The `ApiModule` system is the central mechanism for loading and using native C++ APIs from Lua. It's implemented in `common/lua/scripts/nco/lib/ApiModule.lua`.

### Key Features:
- **Type Validation**: Validates that required parameters are provided and have correct types
- **Mock Support**: Automatically detects and uses mock implementations when available (via `_G.__CNC_API_MOCK`)
- **Read-only Access**: Prevents accidental modification of API modules through metatable protection
- **Error Handling**: Provides detailed error messages when module loading fails

### How It Works:
1. Each API module is defined with an `ApiModuleSpec` containing:
   - `name`: The name of the module (e.g., "Rules", "System")
   - `cppSource`: Location of the C++ source file that implements this API
   - `builder`: Function that creates the Lua table wrapper around the native API

2. When loaded via `ApiModule(moduleSpec)`, it:
   - Validates the parameters using TypeValidator
   - Checks if the required C++ backend is loaded
   - Uses either the real C++ API or a mock (if available)
   - Builds and returns the final module table with read-only protection

## CncApiMock System

The `CncApiMock` system in `common/lua/scripts/nco/lib/CncApiMock.lua` enables testing of Lua scripts without running the full game.

### Key Features:
- **Call Recording**: Captures all calls made to mocked APIs
- **Parameter Capture**: Records parameters passed to functions
- **Test Assertions**: Provides access to call history for assertions in tests
- **Extensibility**: Allows adding new mocked APIs

### How It Works:
1. When loaded, it creates a global `_G.__CNC_API_MOCK` table
2. This mock is detected by `ApiModule` and used instead of real C++ APIs
3. All calls are recorded in the internal `__calls` table for inspection
4. Test scripts can verify behavior by checking call history

## Module Building Pattern

Each API module follows a consistent building pattern:

### 1. Specification (`ApiModuleSpec`)
```lua
{
  name = "Rules",          -- Name used to access from Lua
  cppSource = "common/lua/rules_luaapi.h",  -- C++ implementation location
  builder = RulesApiProxy    -- Function that creates the Lua API table
}
```

### 2. Builder Function
Each module has a builder function (like `RulesApiProxy`) that:
- Takes the native C++ API and spec as parameters
- Creates a Lua table with the desired API interface
- Handles any translation between C++ and Lua

### 3. Module Loading
```lua
local Rules = ApiModule({
  name = "Rules",
  cppSource = "common/lua/rules_luaapi.h",
  builder = RulesApiProxy
})
```

This pattern ensures consistency across all API modules while allowing each to implement its specific functionality.

## Error Handling and Validation

The system provides comprehensive error handling:
- Parameter validation using TypeValidator
- Clear error messages when C++ backends are not loaded
- Detailed errors for builder function failures
- Type checking of return values from API calls
