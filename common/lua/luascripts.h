#pragma once

#include <string>

class LuaScripts final {
public:
  inline static const std::string_view On_Scenario_Load = "on-scenario-load.lua";

private:
  LuaScripts() {}
};
