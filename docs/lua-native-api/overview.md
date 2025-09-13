# Lua Native API Overview

This documentation provides an overview of how the Lua native library works in Command & Conquer games, allowing people to write Lua scripts that run inside the real-time strategy game.

## Architecture

The system consists of:

1. **C++ Game Engine** - Core game logic and functionality
2. **Native C++ Library** - Exposes C++ APIs to Lua scripts
3. **Lua Scripts** - User-written scripts that interact with the native library
4. **API Module System** - The mechanism for loading and using native APIs from Lua

## Core Concepts

### ApiModule Pattern
The `ApiModule` system is the core mechanism for loading native C++ APIs into Lua. It provides:
- Type validation of API module parameters
- Mocking support for testing
- Read-only access to prevent accidental modification
- Error handling and reporting

### CncApiMock
A mocking system that allows developers to test Lua scripts without running the full game engine. The mock captures calls made to the C++ APIs and can be inspected for assertions.

### Module Building Pattern
Each API module follows a pattern where:
1. An `ApiModuleSpec` is defined with name, C++ source location, and builder function
2. A builder function creates the Lua table that wraps the native API
3. The module is loaded using `ApiModule()` which handles validation and mocking

## Game-Specific Extensions

Different game engines (like Tiberian Dawn) extend this base system with additional APIs.

## Documentation Structure

This documentation covers:
1. **Overview & Architecture** - High-level concepts
2. **Core Concepts** - Detailed explanations of key components  
3. **API Reference (Generic)** - How to use the APIs
4. **Game-Specific Extensions** - Additional functionality
5. **Usage Examples** - Concrete implementation examples
6. **Best Practices** - Guidelines for development

The documentation is structured to be generic and conceptual, while concrete implementations are referenced for clarity.
