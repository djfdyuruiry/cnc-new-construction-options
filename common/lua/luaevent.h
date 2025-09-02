#pragma once

#include <string>
#include <variant>

#include "../logger.h"

using LuaEventData = std::variant<std::string, int, double>;

/**
 * Encapsulates a piece of discrete logic that a Lua
 * thread wants to be executed async. Used to pass
 * messages to the game engine.
 */
class LuaEvent {
public:
    LuaEvent(const std::string_view type, const LuaEventData data): EventType(type), Data(data) {}

    virtual ~LuaEvent() = default;
    virtual void Execute() const {
        CNC_LOGGER_DEBUG("Executing lua event of type: {}", EventType);
    };

protected:
    inline static CncLogger Logger = CncLogger("LuaEvent");

    const std::string_view EventType;
    const LuaEventData Data;
};
