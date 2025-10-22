#pragma once

#include <format>
#include <string>

#include "logging_luaapi.h"
#include "luaapi.h"
#include "luaarguments.h"

/**
 * API for calling Lua native event handlers.
 */
class EventLuaApi : public LuaApi
{
public:
    static inline const std::string EventGlobalTable = "Event";
    static inline const std::string HandlersTable = "handlers";
    static inline const std::string HandlersTablePath = std::format("{}.{}", EventGlobalTable, HandlersTable);

    template<LuaPushType... Args>
    static LuaResult Execute_Event(
        const LuaEngine& e,
        std::string_view event_name,
        Args&&... args
    )
    {
        return e.With_Global(EventGlobalTable, LUA_TTABLE, [&]() {
            return e.With_Table_Field(EventGlobalTable, HandlersTable, LUA_TTABLE, [&]() {
                return e.With_Table_Field(HandlersTablePath, event_name, LUA_TFUNCTION, [&]() {
                    return e.PCall_With_Args(
                        std::format("{}.{}", HandlersTablePath, event_name),
                        std::forward<Args>(args)...
                    );
                });
            });
        });
    }

    EventLuaApi() : LuaApi("Event") {}

    virtual void Register_Dependencies(LuaEngine& engine) const override
    {
        engine.Register_Api<LoggingLuaApi>();
    }
};
