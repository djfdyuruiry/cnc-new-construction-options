#pragma once

#include <string>

#include "logger.h"

#include "luaengine.h"

/**
 * Encapsulates a piece of discrete logic that a Lua
 * thread wants to be executed async. Used to pass
 * messages to the game engine.
 */
class LuaEvent
{
public:
    LuaEvent(const std::string_view type): EventType(type) {}

    virtual ~LuaEvent() = default;
    virtual void Execute() const
    {
        CNC_LOGGER_DEBUG("Executing lua event of type: {}", EventType);
    }

    virtual void Execute(const LuaEngine& engine) const
    {
        Execute();
    }

protected:
    static inline const auto& Logger = CncLogger::For(LuaEvent);

    const std::string_view EventType;
};
