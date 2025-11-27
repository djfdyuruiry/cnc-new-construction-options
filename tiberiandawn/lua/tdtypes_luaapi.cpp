#include "../../common/lua/luatablebuilder.h"

#include "../function.h"

#include "tdtypes_luaapi.h"

void TiberianDawnTypesLuaApi::Register_Functions(LuaEngine& engine) const
{
    With_Api_Namespace(engine, [&](auto& n) {
        n.addCFunction("getTypeNames", [](auto L) {
            const auto engine = SharedLuaEngine(L);

            LuaTableBuilder::Push_New_Table(engine)
                .With_Index_Value(TdTypeConverter::Get_Type_Name<AnimType>())
                .With_Index_Value(TdTypeConverter::Get_Type_Name<WarheadType>())
                .With_Index_Value(TdTypeConverter::Get_Type_Name<BulletType>())
                .With_Index_Value(TdTypeConverter::Get_Type_Name<WeaponType>())
                .With_Index_Value(TdTypeConverter::Get_Type_Name<AircraftType>())
                .With_Index_Value(TdTypeConverter::Get_Type_Name<StructType>())
                .With_Index_Value(TdTypeConverter::Get_Type_Name<InfantryType>())
                .With_Index_Value(TdTypeConverter::Get_Type_Name<UnitType>())
                .With_Index_Value(TdTypeConverter::Get_Type_Name<HousesType>());

            return 1;
        });

        Register_Type_Functions<AnimType, AnimTypeClass>(n);
        Register_Type_Functions<WarheadType, WarheadTypeClass>(n);
        Register_Type_Functions<BulletType, BulletTypeClass>(n);
        Register_Type_Functions<WeaponType, WeaponTypeClass>(n);
        Register_Type_Functions<AircraftType, AircraftTypeClass>(n);
        Register_Type_Functions<StructType, BuildingTypeClass>(n);
        Register_Type_Functions<InfantryType, InfantryTypeClass>(n);
        Register_Type_Functions<UnitType, UnitTypeClass>(n);
        Register_Type_Functions<HousesType, HouseTypeClass>(n);
    });
}
