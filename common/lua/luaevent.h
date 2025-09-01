#pragma once

#include <string>
#include <variant>

using LuaEventData = std::variant<std::string, int, double>;

class LuaEvent {
public:
    LuaEvent(const LuaEventData data) : Data(data) {}

    virtual ~LuaEvent() = default;
    virtual void Execute() const = 0;

protected:
    const LuaEventData Data;
};
