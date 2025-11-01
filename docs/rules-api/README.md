# Rules API Documentation

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
## Tiberian Dawn Rules

`CCINIClass` is a sub-class of `INIClass` for Tiberian Dawn, it supports TD types for loading Scenarios and saving from the Scenario Editor (Units, Terrain, Triggers etc.)

`RulesClass` is a class that reads in settings from a `rules.ini` file for game options that effect the game engine globally - AI difficulty, Skirmish Controls and extra sections added by New Construction Options. 

>This class was back-ported from Red Alert as part of the Remastered Collection development, specifically to add Skirmish support.

## `rulesections.h`

In the common library, NCO has added a new set of classes designed to build on top of the existing rules code to make reading large amounts of INI sections and mapping to C++ types easier.  

`RuleSection` class:
- Allows processing entries from an INI file section and store these in memory
- Use of templates and std::variant to allow reading/writing values to a rule section in memory using concrete types found in the game code (rather than calling individual `INIClass` methods)
- Supporting more numeric types other than int and fixed - char/uchar, uint, and float - with value validation on Load from INI based on `std::numeric_limits` and `std::stoX` functions
- Caching of all rules loaded from INI using the STL library, so you can lookup, query and set values at any time (not just when loading rules using `RulesClass`) 
- Validation of string values using `std::function` lambdas (See type converter docs below)
- Fluent interface for chaining method calls to lots of setup in a readable way
- Can write out stored entries to an INI
- A `std::function` callback can be used to run some code every-time the value for an entry is changed  (used to update data that is calculated based on rule values - e.x. `SpecialClass`)
`IniRuleContext` class:
- Wrapper around `RuleSection` to make loading from an INI file easier 
- Fluent interface for loading entries from a section with default values
- Supports loading entries using converters to load more types than the original `CCINIClass` supports
`RuleSections` class:
- Stores multiple `RuleSection` instances to map out an entire INI file in memory
- Dynamic adds new sections on entry value load
- Used in `RulesClass` to store INI file data for `rules.ini` and types (e.g. `infantry.ini`, `unit.ini`)

## Tiberian Dawn Integration

- `rulesections.h` is integrated with Tiberian Dawn inside the `RulesClass` class, the `Sections` member stores all the sections and entries from the `rules.ini` class.
- This is how NCO game rules are loaded and stored, other code in the game engine references `Sections` to look up rule values - the macros in `rules.h` simplify this (e.x. `Get_Int_Rule`)


``

