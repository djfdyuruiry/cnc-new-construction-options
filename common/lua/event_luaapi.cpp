#include "lua/event_luaapi.h"

void EventLuaApi::Register_Dependencies(LuaEngine& engine) const
{
    engine.Register_Api<LoggingLuaApi>();
}

