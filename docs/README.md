This directory holds technical documentation on various New Construction Options features. The purpose of this is for developers wanting to make changes and for the maintainer to track implementation details.

- [Lua C++ API](lua-cpp-api/README.md) - Used to manage Lua and expose game engine functionality to Lua
- [Lua Native API](lua-native-api/README.md) - Provides global tables in Lua for interacting with the Lua C++ API 
- [Rules API](rules-api) - API that manages rules for in-game types, with Lua integration (INI config and changing rules at runtime)
  - [NcoRules Cmake Generator](../cmake/NcoRules.cmake) - Code generation for rules.ini rules (read the block comment at the start of the file)
- [High Resolution Support](hi-resolution/README.md) - Enables running Tiberian Dawn at internal resolutions higher than 640x400
- [[skirimish-with-minimap.jpg]] - Designs for new Skirmish setup screen
- [[Release Test Plan]] - Full test plan with execution steps that can be followed when a new feature is ready for release
