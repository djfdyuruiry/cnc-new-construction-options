#include "event_luaapi.h"
#include "logging_luaapi.h"
#include "luaarguments.h"

void EventLuaApi::Register_Dependencies(LuaEngine& engine) const
{
    engine.Register_Api<LoggingLuaApi>();
}
