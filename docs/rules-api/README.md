# Rules API Documentation

These docs explain how rules are loaded into NCO from INI files and get/set at runtime from C++ code and Lua scripts.
It also includes details on how Types in the game engines are managed (Infantry, Units etc.).

- [Common Library](#common-library)
- [Tiberian Dawn](#tiberian-dawn)
- [Rule Sections](1.rulesections.md)
  - [Type Converters](1.rulesections.md#tiberian-dawn-types)
  - [Tiberian Dawn Integration](1.rulesections.md#tiberian-dawn-integration)
    - [On Change Callbacks](1.rulesections.md#on-change-callback)
    - [TdTypeConverter](1.rulesections.md#tdtypeconverter)
    - [Code Generation](1.rulesections.md#code-generation)
- [Tiberian Dawn Types](2.td-types.md)
  - [Code Generation](2.td-types.md#code-generation)
- [Lua Integration](3.lua-generation.md)
  - [Rules API](3.lua-generation.md#code-generation)
  - [Tiberian Dawn](3.lua-generation.md#tiberian-dawn)
    - [Types API](3.lua-generation.md#types-api)

## Common Library

The original C&C code uses a INI class `INIClass` to parse INI files and fetch rules based on section and entry name (e.x. `[Section] Entry=Value`). This is used for:

- Loading game rules 
- Loading game options and state
- Loading Scenario INI settings for single player missions and multi player maps

`INIClass` supports Serialization and Deserialization of a variety of types including:

- Ints
- Strings
- Fixed (custom floating point type)
- Hex
- Bool
- Text Blocks (multi-line strings)
- Binary data (base64 strings)

## Tiberian Dawn

`CCINIClass` is a sub-class of `INIClass` for Tiberian Dawn, it supports TD types for loading Scenarios and saving from the Scenario Editor (Units, Terrain, Triggers etc.)

`RulesClass` is a class that reads in settings from a `rules.ini` file for game options that effect the game engine globally - AI difficulty, Skirmish Controls and extra sections added by New Construction Options. 

>This class was back-ported from Red Alert as part of the Remastered Collection development, specifically to add Skirmish support.
