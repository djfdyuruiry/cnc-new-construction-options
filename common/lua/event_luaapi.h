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
    inline static const std::string GlobalTable = "Event";
    inline static const std::string HandlersTable = "handlers";

    template<typename... Args>
    static LuaResult Execute_Event(
        const LuaEngine& engine,
        std::string_view event_name,
        Args&&... args
    ) {
        auto lookup_status = false;
        auto lookup_count = 0;

        engine.Load_Global(GlobalTable);
        lookup_status = engine.Is_Table();
        lookup_count++;

        if (lookup_status) {
            engine.Load_Table_Field(HandlersTable);
            lookup_status = engine.Is_Table();
            lookup_count++;
        }

        if (!lookup_status) {
            engine.Pop(lookup_count);

            return LuaResult(
                std::format(
                    "Event API handlers table is missing: {}.{}",
                    GlobalTable,
                    HandlersTable
                )
            );
        }

        engine.Load_Table_Field(event_name);
        lookup_count++;

        auto result = engine.PCall_With_Args(
            std::format(
                "{}.{}.{}",
                GlobalTable,
                HandlersTable,
                event_name
            ),
            std::forward<Args>(args)...
        );

        engine.Pop(lookup_count);

        return result;
    }

    EventLuaApi() : LuaApi("Event", true) {}

    virtual void Register_Dependencies(LuaEngine& engine) const override {
        engine.Register_Api<LoggingLuaApi>();
    }
};