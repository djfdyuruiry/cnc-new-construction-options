# Rules API Documentation

These docs explain how rules are loaded into NCO from INI files and get/set at runtime from C++ code and Lua scripts.
It also includes details on how Types in the game engines are managed (Infantry, Units etc.).

- [Original Code](0.original-code.md)
  - [Common Library](0.original-code.md#common-library)
  - [Tiberian Dawn](0.original-code.md#tiberian-dawn)
- [Rule Sections](1.rulesections.md)
  - [Type Converters](1.rulesections.md#tiberian-dawn-types)
  - [Tiberian Dawn Integration](1.rulesections.md#tiberian-dawn-integration)
    - [On Change Callbacks](1.rulesections.md#on-change-callbacks)
    - [`TdTypeConverter` Class](1.rulesections.md#tdtypeconverter-class)
    - [Code Generation](1.rulesections.md#code-generation)
- [Tiberian Dawn Types](2.td-types.md)
  - [Class Structure](2.td-types.md#class-structure)
    - [Hierarchy](2.td-types.md#hierarchy)
  - [Type Rules](2.td-types.md#type-rules)
  - [On Change Callback](2.td-types.md#on-change-callback)
  - [Code Generation](2.td-types.md#code-generation)
- [Lua Integration](3.lua-integration.md)
  - [Rules API](3.lua-integration.md#rules-api)
  - [Tiberian Dawn](3.lua-integration.md#tiberian-dawn)
    - [Types API](3.lua-integration.md#types-api)
