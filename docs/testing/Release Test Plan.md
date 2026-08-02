# Release Test Plan

This document contains test cases, with execution steps and expected behaviour. Use these cases to validate a new release of New Construction Options. Not all steps may be necessary depending on the changes in the release.

## Test Packs

### Test Pack: Launcher Changes
- Standalone Install (MacOS, Windows, Linux) [TC-001](#TC-001)
- CLI Install (`--install-all`, `--install-td`, `--install-ra`) [TC-002](#TC-002)
- Standalone Data Integrity [TC-003](#TC-003)
- User Data Files: Verify logs, rules, and engine configs [TC-004](#TC-004)

### Test Pack: Save/Load Tiberian Dawn
- Save Integrity (JSON) [TC-005](#TC-005)
- JSON Schema Validation [TC-006](#TC-006)
- Save/Load Data Consistency [TC-007](#TC-007)
- Skirmish Save/Load (JSON & Remaster) [TC-008](#TC-008)
- Campaign Save/Load (JSON & Remaster) [TC-009](#TC-009)
- Legacy Binary Integrity & Data Compatibility [TC-010](#TC-010)
- Legacy Campaign Save/Load (Legacy & Remaster) [TC-011](#TC-011)
- Remaster Legacy Save/Load [TC-012](#TC-012)
- User Data Files: Verify logs, rules, config files (CONQUER.INI), and save files [TC-013](#TC-013)
- Script/Scenario Loading: Verify Lua and Scenario (INI) files [TC-014](#TC-014)

### Test Pack: Save/Load Red Alert
- Save Integrity (JSON) [TC-015](#TC-015)
- JSON Schema Validation [TC-016](#TC-016)
- Save/Load Data Consistency [TC-017](#TC-017)
- Skirmish Save/Load (JSON & Remaster) [TC-018](#TC-018)
- Campaign Save/Load (JSON & Remaster) [TC-019](#TC-019)
- Legacy Binary Integrity & Data Compatibility [TC-020](#TC-020)
- Legacy Campaign Save/Load (Legacy & Remaster) [TC-021](#TC-021)
- Remaster Legacy Save/Load [TC-022](#TC-022)
- User Data Files: Verify logs, rules, config files (REDALERT.INI), and save files [TC-023](#TC-023)
- Script/Scenario Loading: Verify Lua and Scenario (INI) files [TC-024](#TC-024)

### Test Pack: New Tiberian Dawn Feature
- Construction Logic (Placement, Queue, Completion) [TC-025](#TC-025)
- Building Functionality [TC-026](#TC-026)
- Unit Movement and Pathing [TC-027](#TC-027)
- Unit Interaction [TC-028](#TC-028)
- Collision and Obstruction [TC-029](#TC-029)
- Placement Rules (Distance, Modern Wall) [TC-030](#TC-030)
- Game-Specific Mechanics (TD) [TC-031](#TC-031)
- Frame Rate (FPS) [TC-032](#TC-032)
- Memory Usage (Valgrind/Massif) [TC-033](#TC-033)
- Load Times [TC-034](#TC-034)
- High-Unit Count Stress Test [TC-035](#TC-035)
- Crash Reporting [TC-036](#TC-036)
- User Data Files: Verify logs, rules, and config file (CONQUER.INI) [TC-037](#TC-037)

### Test Pack: New Red Alert Feature
- Construction Logic (Placement, Queue, Completion) [TC-038](#TC-038)
- Building Functionality [TC-039](#TC-039)
- Unit Movement and Pathing [TC-040](#TC-040)
- Unit Interaction [TC-041](#TC-041)
- Collision and Obstruction [TC-042](#TC-042)
- Placement Rules (Distance, Modern Wall) [TC-043](#TC-043)
- Game-Specific Mechanics (RA) [TC-044](#TC-044)
- Frame Rate (FPS) [TC-045](#TC-045)
- Memory Usage (Valgrind/Massif) [TC-046](#TC-046)
- Load Times [TC-047](#TC-047)
- High-Unit Count Stress Test [TC-048](#TC-048)
- Crash Reporting [TC-049](#TC-049)
- User Data Files: Verify logs, rules, and config file (REDALERT.INI) [TC-050](#TC-050)

### Test Pack: Hi-Res UI Change
- Video Mode - DOS (Fixed map/sidebar) [TC-051](#TC-051)
- Video Mode - Standard (Fixed map/sidebar) [TC-052](#TC-052)
- Video Mode - Hi-Res (Dynamic map/sidebar) [TC-053](#TC-053)
- Windowed Mode (Scaling, UI, aspect ratio) [TC-054](#TC-054)
- Fullscreen Mode (Scaling, UI, aspect ratio) [TC-055](#TC-055)
- Hi-Res - Original Content Rendering (Centered/Zoom) [TC-056](#TC-056)
- Cross-Mode Save/Load (Hi-Res Rehydration) [TC-057](#TC-057)
- User Data Files: Verify logs, rules, and config files [TC-058](#TC-058]

### Test Pack: Menu UI Changes
- UI Layout Consistency (DOS) [TC-059](#TC-059)
- UI Layout Consistency (Standard) [TC-060](#TC-060)
- UI Layout Consistency (Hi-Res) [TC-061](#TC-061)
- Windowed/Fullscreen UI scaling and responsiveness [TC-062](#TC-062)
- User Data Files: Verify logs, rules, and config files [TC-063](#TC-063]

## Test Case Details

### Installation & Setup
### TC-001
- **TC-001: Standalone Install**: Run NCO Launcher on target OS. Verify files are extracted to correct path and game binaries are executable.
### TC-002
- **TC-002: CLI Install**: Run launcher with `--install-all`, `--install-td`, and `--install-ra`. Verify correct data is installed for each.
### TC-003
- **TC-003: Standalone Data Integrity**: Verify the extracted folder contains all required assets (maps, sprites, audio, etc.) and is not corrupted.
### TC-004
- **TC-004: User Data Files (General)**: Verify that the game generates `logs` and `rules` files in the correct user directory.

### Save/Load (Tiberian Dawn)
### TC-005
- **TC-005: Save Integrity (JSON)**: Save a game state and reload. Verify all units and buildings are present.
### TC-006
- **TC-006: JSON Schema Validation**: Open a saved JSON file and verify it adheres to the project's JSON schema.
### TC-007
- **TC-007: Save/Load Data Consistency**: Verify unit health, position, and resources match exactly after loading.
### TC-008
- **TC-008: Skirmish Save/Load (JSON & Remaster)**: Verify Skirmish save/load works in both Standalone and Remaster.
### TC-009
- **TC-009: Campaign Save/Load (JSON & Remaster)**: Verify Campaign save/load works in both Standalone and Remaster.
### TC-010
- **TC-010: Legacy Binary Integrity**: Verify that old binary save files can be successfully loaded.
### TC-011
- **TC-011: Legacy Campaign Save/Load**: Verify legacy campaign data is interpreted correctly.
### TC-012
- **TC-012: Remaster Legacy Save/Load**: Verify Remaster-specific legacy saves load correctly.
### TC-013
- **TC-013: User Data Files (Tiberian Dawn)**: Verify `CONQUER.INI` and save files are in the correct folder.
### TC-014
- **TC-014: Script/Scenario Loading**: Verify Lua scripts and Scenario INI files load from the expected user directory.

### Save/Load (Red Alert)
### TC-015
- **TC-015: Save Integrity (JSON)**: Save and reload a Red Alert state.
### TC-016
- **TC-016: JSON Schema Validation**: Verify Red Alert JSON schema.
### TC-017
- **TC-017: Save/Load Data Consistency**: Verify RA values are accurate after loading.
### TC-018
- **TC-018: Skirmish Save/Load (JSON & Remaster)**: Verify RA Skirmish save/load.
### TC-019
- **TC-019: Campaign Save/Load (JSON & Remaster)**: Verify RA Campaign save/load.
### TC-020
- **TC-020: Legacy Binary Integrity**: Verify RA legacy binary files load.
### TC-021
- **TC-021: Legacy Campaign Save/Load**: Verify legacy RA campaign data.
### TC-022
- **TC-022: Remaster Legacy Save/Load**: Verify Remaster RA legacy saves.
### TC-023
- **TC-023: User Data Files (Red Alert)**: Verify `REDALERT.INI` and save files are in the correct folder.
### TC-024
- **TC-024: Script/Scenario Loading**: Verify Lua and Scenario INI files load for RA.

### Gameplay Mechanics (Tiberian Dawn)
### TC-025
- **TC-025: Construction Logic**: Verify buildings can be placed, queued, and completed correctly.
### TC-026
- **TC-026: Building Functionality**: Verify buildings provide expected power/production.
### TC-027
- **TC-027: Unit Movement and Pathing**: Verify units move to targets and navigate obstacles.
### TC-028
- **TC-028: Unit Interaction**: Verify units attack/move to locations.
### TC-029
- **TC-029: Collision and Obstruction**: Verify entities occupy space correctly.
### TC-030
- **TC-030: Placement Rules**: Verify distance limits and "modern wall" behavior.
### TC-031
- **TC-031: Game-Specific Mechanics (TD)**: Verify TD-specific NCO features.
### TC-032
- **TC-032: Frame Rate (FPS)**: Check FPS during standard gameplay.
### TC-033
- **TC-033: Memory Usage (Valgrind/Massif)**: Run Massif and verify memory leaks/growth.
### TC-034
- **TC-034: Load Times**: Measure initialization/level load times.
### TC-035
- **TC-035: High-Unit Count Stress Test**: Verify stability with many active units.
### TC-036
- **TC-036: Crash Reporting**: Verify logs are written on a forced crash.
### TC-037
- **TC-037: User Data Files (Tiberian Dawn)**: Verify `CONQUER.INI` generation.

### Gameplay Mechanics (Red Alert)
### TC-038
- **TC-038: Construction Logic**: Verify RA building placement and queue.
### TC-039
- **TC-039: Building Functionality**: Verify RA building production.
### TC-040
- **TC-040: Unit Movement and Pathing**: Verify RA unit pathing.
### TC-041
- **TC-041: Unit Interaction**: Verify RA unit interaction.
### TC-042
- **TC-042: Collision and Obstruction**: Verify RA entity collision.
### TC-043
- **TC-043: Placement Rules**: Verify RA distance/modern wall behavior.
### TC-044
- **TC-044: Game-Specific Mechanics (RA)**: Verify RA-specific NCO features.
### TC-045
- **TC-045: Frame Rate (FPS)**: Check RA FPS.
### TC-046
- **TC-046: Memory Usage (Valgrind/Massif)**: Run Massif for RA.
### TC-047
- **TC-047: Load Times**: Measure RA load times.
### TC-048
- **TC-048: High-Unit Count Stress Test**: Verify RA stability with many units.
### TC-049
- **TC-049: Crash Reporting**: Verify RA crash logs.
### TC-050
- **TC-050: User Data Files (Red Alert)**: Verify `REDALERT.INI` generation.

### Graphics & UI (Hi-Res & Modes)
### TC-051
- **TC-051: Video Mode - DOS**: Verify fixed map/sidebar and rendering.
### TC-052
- **TC-052: Video Mode - Standard**: Verify fixed map/sidebar and rendering.
### TC-053
- **TC-053: Video Mode - Hi-Res**: Verify dynamic map/sidebar and rendering.
### TC-054
- **TC-054: Windowed Mode**: Verify scaling, UI, and aspect ratio.
### TC-055
- **TC-055: Fullscreen Mode**: Verify scaling, UI, and aspect ratio.
### TC-056
- **TC-056: Hi-Res - Original Content**: Verify centered/zoom for movies/menus.
### TC-057
- **TC-057: Cross-Mode Save/Load**: Verify Hi-Res rehydration of saved data.
### TC-058
- **TC-058: User Data Files**: Verify logs/rules/configs in correct folder.

### Menu UI Changes
### TC-059
- **TC-059: UI Layout Consistency (DOS)**: Verify DOS UI layout.
### TC-060
- **TC-060: UI Layout Consistency (Standard)**: Verify Standard UI layout.
### TC-061
- **TC-061: UI Layout Consistency (Hi-Res)**: Verify Hi-Res UI layout.
### TC-062
- **TC-062: Windowed/Fullscreen UI**: Verify scaling and responsiveness.
### TC-063
- **TC-063: User Data Files**: Verify logs/rules/configs in correct folder.
