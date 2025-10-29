#include "../../function.h"

#include "addtrigger_luaevent.h"

AddTriggerLuaEvent::AddTriggerLuaEvent(std::string name, std::string definition): LuaEvent("AddTrigger") {
    Name = std::move(name);
    Definition = std::move(definition);
}

void AddTriggerLuaEvent::Execute() const {
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