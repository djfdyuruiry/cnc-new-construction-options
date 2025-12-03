# Tiberian Dawn JSON Save Game Format

## Overview

This document outlines a new JSON-based save game format for Command & Conquer: Tiberian Dawn. The JSON format provides better versioning support, easier editing of saved games, and more robust handling of defaults.

## Current Binary Save System

The existing system saves games as binary files that record pointer data directly. Key components include:

- Map data (cells, terrain)
- Object instances (houses, units, buildings)
- Game state variables
- Pointer encoding/decoding for compatibility

The game engine uses the concept of 'encoding' and 'decoding' pointers, essentially what this means is:

- During encoding, any member of a class that is a pointer type is replaced with a special type: `TARGET_COMPOSITE`
- It holds the type info as a union that allows mapping a target, mantissa and exponent
- This target is intended to be cast to `TARGET` (an int), so compatible with the storage format for pointers (4 byte int pointing to a memory address)
- The Exponent holds the RTTI type of the pointer (Building, Overlay etc.) and the Mantissa holds the instance ID (enum value)
- When decoding this union is used to determine the type and fetch the current valid pointer address for the instance

The implications of this are:

- Type instance IDs must be present in the game engine for loading a saved game to work as expected (instance IDs are fixed)
- The binary format of a save is directly coupled to the class structure of game types (class can't change)
- `XTypeClass` structures can be different, these are not recorded in the save (but again coupled to enum ID)
- Save games shared between platforms might not be compatible, if storage sizes for C++ types differ between those platforms (binary format is platform specific - x86 WIN32 MSVC for example)

## Proposed JSON Format

```json5
{
  "header": {
    "version": 1,
    "description": "Player's description",
    "scenario_id": 10,
    "player_house": "GDI",
    "difficulty": "normal"
  },
  "map": {
    "dimensions": {
      "width": 256,
      "height": 256
    },
    "cells": [
      {
        "x": 0, "y": 0, "terrain_type": "grass", "occupier_id": null, // ...
      }
    ],
    "layers": [ /* ... */ ]
  },
  "houses": [
    {
      "id": 0,
      "type": "GDI",
      "name": "Player1",
      "credits": 5000,
      "power": 200,
      "units": [ /* references */ ],
      "buildings": [ /* references */ ]
    }
  ],
  "objects": {
    "units": [
      {
        "id": 1,
        "type": "harvester",
        "position": { "x": 100, "y": 200 },
        "health": 80,
        "owner_id": 0,
        "status": "active"
      }
    ],
    "buildings": [
      {
        "id": 2,
        "type": "construction_yard",
        "position": { "x": 50, "y": 60 },
        "health": 100,
        "owner_id": 0
      }
    ]
  },
  "scenario_data": {
    "waypoints": [ /* ... */ ],
    "briefing_text": "...",
    "rules": { /* ... */ },
    "scenario_name": "Mission 1"
  },
  "game_state": {
    "selected_objects": [1, 2],
    "frame_count": 4567,
    "score": { /* ... */ }
  }
}
```

## Key Benefits

1. **Human-Readable**: Easier to edit and understand
2. **Versioning Support**:
   - Version number in header
   - Migration paths between versions
3. **Schema Validation**: Define defaults and required fields
4. **Extensibility**: Easy to add new fields with backward compatibility

## Implementation Plan

### Phase 1: Design & Documentation

- [x] Analyze current save system
- [x] Design JSON schema (above)
- [x] Document migration strategy

### Phase 2: Infrastructure

- [x] Choose JSON library (nlohmann/json available via vcpkg)
- [ ] Create JSON schema validation
- [ ] Implement default value handling

### Phase 3: Implementation

- [ ] Create JSON save/load functions
- [ ] Handle binary to JSON migration
- [ ] Implement version migration logic
- [ ] Update Save_Game/Load_Game functions

## JSON Serialization Example

The nlohmann/json library provides excellent support for arbitrary types through its serialization features. Here's how we can implement JSON serialization for the `AircraftClass`:

See: https://json.nlohmann.me/features/arbitrary_types/

```cpp
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class AircraftClass : public FootClass, public FlyClass {
    // ... existing members ...

public:
    // ... existing methods ...

    // JSON serialization methods
    NLOHMANN_JSON_SERIALIZE_ENUM(
        FacingType,
        {FACING_NONE, FACING_NORTHWEST, FACING_NORTH, FACING_NORTHEAST,
         FACING_WEST, FACING_EAST, FACING_SOUTHWEST, FACING_SOUTH, FACING_SOUTHEAST}
    )

    NLOHMANN_JSON_SERIALIZE_ENUM(
        DirType,
        {DIR_NONE, DIR_NORTHWEST, DIR_NORTH, DIR_NORTHEAST,
         DIR_WEST, DIR_EAST, DIR_SOUTHWEST, DIR_SOUTH, DIR_SOUTHEAST}
    )

    // Custom serialization for AircraftClass
    friend void to_json(json& j, const AircraftClass& a) {
        j = json{
            {"type", "aircraft"},
            {"class_id", a.Class->Type},
            {"house", a.Owner()},
            {"position", {a.x_Position(), a.y_Position()}},
            {"altitude", a.Altitude},
            {"health", a.Health()},
            {"facing", a.facing},
            {"secondary_facing", static_cast<int>(a.SecondaryFacing)},
            {"attacks_remaining", a.AttacksRemaining}
        };
    }

    friend void from_json(const json& j, AircraftClass& a) {
        if (j.at("type") != "aircraft")
            throw std::runtime_error("Invalid aircraft data");

        a.Class = AircraftTypeClass::As_Reference(static_cast<AircraftType>(j.at("class_id").get<int>()));
        // ... set other fields ...
    }
};
```

## Serialization Approach

1. **Enum Serialization**: Use `NLOHMANN_JSON_SERIALIZE_ENUM` for all game enums
2. **Custom Serialization**: Implement `to_json` and `from_json` friends for each class
3. **Type Safety**: Include type fields in JSON to validate during deserialization
4. **Error Handling**: Validate JSON data structure during loading

This approach provides:
- Type safety through the JSON library's automatic conversion
- Extensibility with easy addition of new fields
- Validation capabilities during deserialization

## Migration Strategy (Separate Tool)

Migration from binary format to JSON will be handled by a dedicated conversion tool rather than being integrated into the core game engine. This approach allows for:

1. **Clean Separation**: Core save functionality won't need dual-format support
2. **Focused Development**: Conversion logic can be developed and tested independently
3. **User Control**: Players can choose when to convert their saves

The conversion tool will:
- Parse binary .sav files
- Convert to JSON format with proper schema validation
- Handle version-specific migration paths
- Maintain all game data accurately

**Note**: The core implementation will focus on JSON format for new saves only. Binary save support remains unchanged until the dedicated conversion tool is available.

## Current Save System Analysis

Based on `tiberiandawn/saveload.cpp`:

1. **Save_Game Process**:
   - Header info (description, scenario, house)
   - Map save (calls DisplayClass::Save → CellClass::Write)
   - Object saves through ArrayOf classes
   - Layer saves (Logic and Map layers)
   - House saves (credits, power, etc.)
   - Miscellaneous variables

2. **Key Classes to Serialize**:
   - `HouseClass`: credits, power, buildings, units
   - `ObjectClass`: base class for all game objects
   - `MapClass`: map dimensions, cells, layers
   - `CellClass`: terrain, occupiers, triggers

3. **Save Order**: The current system saves:
   1. Map first (needed for theater initialization)
   2. Game objects through ArrayOf classes
   3. Logic and map layers
   4. Score, AI base, misc variables

This design provides a foundation for migrating to JSON while maintaining compatibility with existing save files.
