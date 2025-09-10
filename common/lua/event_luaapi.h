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
        return engine.Get_Value_From_State<LuaResult>([&](auto L) {
            lua_getglobal(L, GlobalTable.c_str()); // get event table
            lua_getfield(L, -1, HandlersTable.c_str()); // get handlers table
            lua_getfield(L, -1, event_name.data()); // get event handler function

            if (!lua_isfunction(L, -1)) {
                lua_pop(L, 3);

                return LuaResult(
                    std::format(
                        "Event handler is either missing or not a function: {}.{}.{}",
                        GlobalTable,
                        HandlersTable,
                        event_name
                    )
                );
            }

            // push args to lua stack
            engine.Push_Values(std::forward<Args>(args)...);

            auto result = LuaResult(
                L,
                lua_pcall(L, sizeof...(Args), 1, 0)
            );

            lua_pop(L, 3);

            return result;
        });
    }

    EventLuaApi() : LuaApi("Event", true) {}

    virtual void Register_Dependencies(LuaEngine& engine) const override {
        engine.Register_Api<LoggingLuaApi>();
    }
};