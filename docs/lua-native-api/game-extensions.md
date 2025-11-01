# Game-Specific Extensions

This document covers how different game engines extend the base Lua native API system with additional functionality.

## Tiberian Dawn Extensions

The Tiberian Dawn game engine provides additional APIs that extend the base functionality available in other games. These extensions are located in `tiberiandawn/lua/scripts/nco/TiberianDawn/lib/`.

### Key Extensions

#### Game API (`Game`)
Provides game state and control functions specific to Tiberian Dawn:
- Win/Lose conditions
- Scenario management
- Game state monitoring

#### Messages API (`Messages`)  
Provides messaging capabilities:
- Sending messages to players
- Message formatting and delivery

#### Scenario API (`Scenario`)
Provides scenario-specific functionality:
- Scenario information access (name, type, faction)
- House management (money, names)
- Team type management
- Trigger system manipulation

#### UI API (`Types`)
Manage properties for game types (Units, Infantry etc.):
- Look up names of a Types
- Look up instance names for a given Type (e.x. E1, E2)
- Look up property names for a given Type (same as those in the INI files)
- Get/Set properties for a given Type instance

#### UI API (`UI`)
Provides user interface functions:
- Popup dialogs
- User interaction handling

### Extension Pattern

Tiberian Dawn extensions follow the same pattern as base functionality but with additional modules:
1. Extend the mock system with Tiberian Dawn specific calls
2. Add new builder functions for the extended APIs
3. Provide access through global variables like other APIs

## Implementation Details

The extensions are implemented in:
- `tiberiandawn/lua/scripts/nco/TiberianDawn/lib/TdApiModule.lua` - Extends base ApiModule
- `tiberiandawn/lua/scripts/nco/TiberianDawn/lib/TdCncApiMock.lua` - Provides Tiberian Dawn specific mocking

## Usage Pattern

```lua
-- Access extended Tiberian Dawn APIs
Game    -- Tiberian Dawn game control
Messages  -- Message system
Scenario  -- Scenario management
Types     -- Game types property management
UI        -- User interface functions

-- Use extended functionality
Game.win()                    -- Win the game
Messages.sendToPlayer("Hello") -- Send message to player
Scenario.modifyHouseMoney(1000) -- Modify house money
Types.Unit.HTNK.Primary = "OBELISK_LASER" -- Give Mammoth Tank an Oblilisk Laser
UI.popupOk("Message")           -- Show popup
```

## Differences from Base API

The Tiberian Dawn extensions:
- Provide game-specific functionality not available in other engines
- Add new global variables for accessing extended APIs
- Include enhanced mocking capabilities specific to Tiberian Dawn features
- May include additional error handling specific to the game engine
