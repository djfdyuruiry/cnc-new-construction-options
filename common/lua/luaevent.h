#pragma once

#include <string>
#include <variant>

#include "../logger.h"

using LuaEventData = std::variant<std::string, int, double>;

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
