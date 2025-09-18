#pragma once 

#include "../../../common/lua/luaevent.h"

#include "../../trigger.h"

class DeleteTriggerLuaEvent : public LuaEvent
{
public:
    DeleteTriggerLuaEvent(
        std::string name
    ) : LuaEvent("DeleteTrigger")
    {
        Name = name;
    }

    virtual void Execute() const override
    {
        LuaEvent::Execute();
        
        auto trigger = TriggerClass::As_Pointer(Name.c_str());
        auto trigger_exists = trigger != NULL;

        if (!trigger_exists) {
          CNC_LOGGER_WARN("Ignoring DeleteTrigger event as trigger doesn't exist: {}", Name);
          return;
        }

        CNC_LOGGER_DEBUG("Removing scenario trigger: {}", Name);

        trigger->Remove();
        delete trigger;
    }

private:
    std::string Name;
};
