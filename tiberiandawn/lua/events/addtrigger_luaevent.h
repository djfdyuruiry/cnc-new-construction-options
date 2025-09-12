#pragma once 

#include "../../../common/lua/luaevent.h"

#include "../../defines.h"
#include "../../trigger.h"

class AddTriggerLuaEvent : public LuaEvent
{
public:
    AddTriggerLuaEvent(
        // event params
        std::string name,
        std::string definition
    ) : LuaEvent("AddTrigger")
    {
        // store params as event data
        Name = name;
        Definition = definition;
    }

    virtual void Execute() const override
    {
        LuaEvent::Execute();
        CNC_LOGGER_DEBUG(
            "Loading scenario trigger '{}' from Lua call, CSV definition: {}",
            Name,
            Definition
        );

        auto trigger = new TriggerClass();

        trigger->Fill_In(
            Name.c_str(),
            Definition.c_str()
        );

        if (trigger->House != HOUSE_NONE) {
            if (trigger->Action == TriggerClass::ActionType::ACTION_ALLOWWIN) {
                HouseClass::As_Pointer(trigger->House)->Blockage++;
            }
            HouseTriggers[trigger->House].Add(trigger);
            trigger->AttachCount++;
        }
    }

private:
    // event data
    std::string Name;
    std::string Definition;
};
