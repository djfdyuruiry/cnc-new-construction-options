#pragma once

#include <string>

class LuaScripts final
{
public:
  inline static const std::string On_Scenario_Load = "on-scenario-load.lua";

private:
  LuaScripts() = delete;
};
