# Lua Native API Documentation

This is the complete documentation for the Lua native library used in the NCO Command & Conquer engine, allowing people to write Lua scripts running inside the real-time strategy game.

## Table of Contents

1. [Overview & Architecture](overview.md)
2. [Core Concepts](core-concepts.md)
3. [API Reference (Generic)](api-reference.md)
4. [Game-Specific Extensions](game-extensions.md)
5. [Usage Examples & Best Practices](examples-best-practices.md)

## Introduction

This documentation explains how to use the Lua native library that allows developers to write scripts that run inside Command & Conquer games. The system provides access to various aspects of the game engine through a set of APIs exposed from C++ to Lua.

The documentation is organized into conceptual sections:
- **Overview**: High-level architecture and concepts
- **Core Concepts**: Detailed explanations of key mechanisms
- **API Reference**: Generic usage patterns without implementation details
- **Game Extensions**: Game-specific API additions
- **Examples & Best Practices**: Practical usage and development guidelines

## How to Use This Documentation

This documentation is structured to be generic, showing concepts and links rather than concrete implementations. Concrete implementations can be referenced in the source code at:
- `common/lua/scripts/nco/lib` - Core library files
- `common/lua/scripts/nco` - Base API implementations  
- `tiberiandawn/lua/scripts/nco/TiberianDawn/lib` - Tiberian Dawn extensions

The examples provided are illustrative and reference the actual implementation patterns found in the codebase.
