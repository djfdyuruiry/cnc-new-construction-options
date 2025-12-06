#pragma once 

#include "common/lua/luaevent.h"

#include "house.h"

class ModifyHouseMoneyLuaEvent : public LuaEvent
{
public:
    ModifyHouseMoneyLuaEvent(HousesType house_type, int money_modifier);

    void Execute() const override;

private:
    HousesType HouseType;
    int MoneyModifier;
};
