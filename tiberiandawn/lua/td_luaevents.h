#pragma once 

#include "../common/lua/luaevent.h"

#include "../msgbox.h"

class PopupLuaEvent : public LuaEvent {
public:
    PopupLuaEvent(std::string message) : LuaEvent("Popup") {
        Message = message;
    }

    virtual void Execute() const override {
        LuaEvent::Execute();

        WWMessageBox().Process(Message.c_str());
    }

private:
    std::string Message;
};

class ModifyHouseMoneyLuaEvent : public LuaEvent {
public:
    ModifyHouseMoneyLuaEvent(HousesType house_type, int money_modifier) : LuaEvent("ModifyHouseCredits") {
        House_Type = house_type;
        Money_Modifier = money_modifier;
    }

    virtual void Execute() const override {
        LuaEvent::Execute();

        auto house = HouseClass::As_Pointer(House_Type);

        if (Money_Modifier < 0)
        {
            // event wants to take money away from the house
            auto money_to_spend = -Money_Modifier;

            if (house->Available_Money() - money_to_spend > -1) {
                house->Spend_Money(money_to_spend);
            }
        }
        else if (Money_Modifier > 0)
        {
            // event wants to give money to the house
            house->Refund_Money(Money_Modifier);
        }
    }

private:
    HousesType House_Type;
    int Money_Modifier;
};
