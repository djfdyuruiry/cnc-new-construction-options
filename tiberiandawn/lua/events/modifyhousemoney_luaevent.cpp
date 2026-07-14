#include "function.h"

#include "modifyhousemoney_luaevent.h"

ModifyHouseMoneyLuaEvent::ModifyHouseMoneyLuaEvent(HousesType house_type, int money_modifier): LuaEvent("ModifyHouseMoney")
{
    HouseType = house_type;
    MoneyModifier = money_modifier;
}

void ModifyHouseMoneyLuaEvent::Execute() const
{
    LuaEvent::Execute();

    const auto house = HouseClass::As_Pointer(HouseType);

    if (house == nullptr) {
        CNC_LOG_ERROR("Failed to resolve house type instance: {}", HouseType);
        return;
    }

    if (MoneyModifier < 0)
    {
        // event wants to take money away from the house
        const auto money_to_spend = -MoneyModifier;

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
