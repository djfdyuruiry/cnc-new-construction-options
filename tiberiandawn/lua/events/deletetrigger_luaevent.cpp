#include "function.h"

#include "deletetrigger_luaevent.h"

DeleteTriggerLuaEvent::DeleteTriggerLuaEvent(std::string name): LuaEvent("DeleteTrigger")
{
    Name = std::move(name);
}

void DeleteTriggerLuaEvent::Execute() const
{
    LuaEvent::Execute();

    auto trigger = TriggerClass::As_Pointer(Name.c_str());
    const auto trigger_exists = trigger != NULL;

    if (!trigger_exists) {
      CNC_LOGGER_WARN("Ignoring DeleteTrigger event as trigger doesn't exist: {}", Name);
      return;
    }

    CNC_LOGGER_DEBUG("Removing scenario trigger: {}", Name);

    trigger->Remove();
    delete trigger;
}