# Usage Examples & Best Practices

This document provides concrete examples of how to use the Lua native APIs, along with best practices for development.

## Usage Examples

### Basic Rules Access
```lua
-- Access and modify game rules
local buildDistance = Rules["Game.Map"].MaxBuildDistance
Rules["Game.Map"].MaxBuildDistance = 5

-- Read-only section access
-- Rules.MySection = {}  -- Would throw read-only error
```

### System Path Operations
```lua
-- Get system paths
local gamePath = System.gamePath
local luaPath = System.luaPath

-- Create a new path object
local newPath = System.Path("maps/scenario.ini")

-- Open files using system paths
local file, err = System.openGameFile("maps/scenario.ini", "r")
if file then
    -- Process file
    file:close()
end
```

### Testing with Mocking
```lua
-- In a test script, enable mocking
require("nco.lib.CncApiMock")()

-- Now all API modules will use mocks
local rules = Rules
rules["Game.Map"].MaxBuildDistance = 10

-- Verify calls were made (in test)
local calls = _G.__CNC_API_MOCK().__calls()
assert(calls.Rules.setRuleValue[1][1] == "Game.Map")
```

### Tiberian Dawn Extensions Usage
```lua
-- Win the game in Tiberian Dawn
Game.win()

-- Send a message to player
Messages.sendToPlayer("Mission accomplished!")

-- Modify scenario information
Scenario.modifyHouseMoney(1000)
```

### Real Script Examples from tiberiandawn/lua/scripts

Based on actual scripts found in the codebase:

#### Scenario Triggers and Events
```lua
-- Example from scg01ea.lua
local function triggerLookups()
  Logger.debug("Triggers: %s", Utils.arrayToCsv(Scenario.triggers.getNames()))
  Logger.debug("WIN trigger: %s", Scenario.triggers.WIN)
end

local function editingTriggers()
  -- Delete a trigger if it exists
  if not Scenario.triggers.deleteIfExists("RNF4") then
    Logger.warning("Attempted to delete missing trigger: RNF4")
  end

  -- Add a new trigger
  Scenario.triggers.RNF4 = "Time,Lua Event,15,GoodGuy,onTimerTrigger,0"
end

-- Register event handlers
Event.handlers.onTimerTrigger = function(triggerName)
  Logger.debug("Handling trigger %s", triggerName)
  if triggerName == "RNF4" then
    Logger.debug("RNF4 second trigger execute")
  end
end
```

#### Team Type Management
```lua
-- Example from scg01ea.lua
local function addingNewTeam()
  Scenario.teams.LUA1 = "GoodGuy,0,0,0,0,0,7,3,0,0,2,HTNK:1,LST:1,0,1,1"
  Scenario.triggers.TMR4 = "Time,Reinforce.,8,GoodGuy,LUA1,0"
end

local function teamTypeLookups()
  Logger.debug("Team Types: %s", Utils.arrayToCsv(Scenario.teams.getNames()))
  Logger.debug("Team GDIR2: %s", Scenario.teams.GDIR2)
end
```

#### Scenario Information Access
```lua
-- Example from on-scenario-load.lua
Logger.info(
  "Someone started playing %s scenario %s - as faction %s and house %s",
  Scenario.type,
  Scenario.name,
  Scenario.player.faction,
  Scenario.player.house.name
)
```

## Best Practices

### 1. Module Structure
Organize scripts with clear separation of concerns:
```lua
-- Good: Separation of concerns
local Rules = require("nco.Rules")
local System = require("nco.System")

-- Use in script
local function setupGameRules()
    Rules["Game.Map"].MaxBuildDistance = 5
end

local function getGamePath()
    return System.gamePath
end
```

### 2. Error Handling
Always consider error handling for API calls:
```lua
local success, result = pcall(function()
    return Rules["Section"].RuleName
end)

if not success then
    -- Handle error appropriately
    print("Error accessing rule:", result)
end
```

### 3. Testing Strategy
Use the mocking system to test functionality without running the game:
```lua
-- Test script
require("nco.lib.CncApiMock")()

-- Setup mock expectations
local mock = _G.__CNC_API_MOCK()
mock.__extend(function(calls)
    calls.Rules.getRuleValue = {}
end)

-- Execute code under test
Rules["Game.Map"].MaxBuildDistance = 10

-- Verify interactions
local calls = mock.__calls()
assert(#calls.Rules.setRuleValue == 1)
```

### 4. Performance Considerations
- Minimize API calls in tight loops
- Cache frequently accessed values
```lua
-- Good: Cache value
local maxBuildDistance = Rules["Game.Map"].MaxBuildDistance

for i = 1, 100 do
    -- Use cached value instead of repeated API access
    if checkDistance(maxBuildDistance) then
        -- process
    end
end
```

### 5. Documentation and Comments
Document your scripts with clear comments:
```lua
-- Set maximum build distance for map
Rules["Game.Map"].MaxBuildDistance = 5

-- Enable building in shroud
Rules["Game.Map"].PreventBuildingInShroud = false
```

## Common Patterns

### Pattern: Configuration Loading
```lua
local function loadConfiguration()
    local config = {}
    
    -- Load configuration from rules
    config.maxBuildDistance = Rules["Game.Map"].MaxBuildDistance
    config.preventShroud = Rules["Game.Map"].PreventBuildingInShroud
    
    return config
end
```

### Pattern: File Operations
```lua
local function processScenarioFile()
    local scenarioPath = System.gamePath / "maps" / "scenario.ini"
    
    -- Open file using system API
    local file, err = System.openGameFile("maps/scenario.ini", "r")
    if not file then
        error("Failed to open scenario file: " .. err)
    end
    
    -- Process file contents...
    file:close()
end
```

### Pattern: Event Handling (if available)
```lua
-- Setup event listeners
Event.Register("GameStarted", function()
    print("Game has started")
end)

Event.Register("PlayerWin", function(playerName)
    print("Player " .. playerName .. " won!")
end)

-- Handle specific triggers
Event.handlers.onTimerTrigger = function(triggerName)
    Logger.debug("Handling trigger %s", triggerName)
end
