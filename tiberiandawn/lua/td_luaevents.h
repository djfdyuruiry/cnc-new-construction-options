#pragma once 

#include "../common/lua/luaevent.h"

#include "../msgbox.h"

class PopupLuaEvent : public LuaEvent
{
public:
    PopupLuaEvent(std::string message) : LuaEvent("Popup")
    {
        Message = message;
    }

    virtual void Execute() const override
    {
        LuaEvent::Execute();

        WWMessageBox().Process(Message.c_str());
    }

private:
    std::string Message;
};

class ModifyHouseMoneyLuaEvent : public LuaEvent
{
public:
    ModifyHouseMoneyLuaEvent(HousesType house_type, int money_modifier) : LuaEvent("ModifyHouseCredits")
    {
        HouseType = house_type;
        MoneyModifier = money_modifier;
    }

    virtual void Execute() const override
    {
        LuaEvent::Execute();

        auto house = HouseClass::As_Pointer(HouseType);

        if (MoneyModifier < 0)
        {
            // event wants to take money away from the house
            auto money_to_spend = -MoneyModifier;

            if (house->Available_Money() - money_to_spend > -1) {
                house->Spend_Money(money_to_spend);
            }
        }
        else if (MoneyModifier > 0)
        {
            // event wants to give money to the house
            house->Refund_Money(MoneyModifier);
        }
    }

private:
    HousesType HouseType;
    int MoneyModifier;
};
