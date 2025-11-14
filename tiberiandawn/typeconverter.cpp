#include "function.h"
#include "typeconverter.h"

#define ENUM_TYPE_PAIR(TYPE, ...) { Get_Type_Name<TYPE>(), EnumTypeInfo<TYPE>(__VA_ARGS__) }

/**
 * Tables to allow mapping between INI strings and enum values, in both directions.
 */
static const TwoWayMap<AircraftType, std::string> AircraftPatchTable = {{ AIRCRAFT_TRANSPORT, "TRAN" }, { AIRCRAFT_HELICOPTER, "HELI" }, { AIRCRAFT_CARGO, "C17" }};
static const TwoWayMap<AnimType, std::string> AnimPatchTable = {{ ANIM_GRENADE, "GRENADEA" }, { ANIM_SMOKE_PUFF, "SMOKEY" }, { ANIM_FIRE_SMALL, "FIRE_S" }, { ANIM_FIRE_MED, "FIRE_M" }, { ANIM_FIRE_MED2, "FIRE_ME" }, { ANIM_FIRE_TINY, "FIRE_T" }, { ANIM_MUZZLE_FLASH, "GUNFIRE" }, { ANIM_BURN_SMALL, "BURN_S" }, { ANIM_BURN_MED, "BURN_M" }, { ANIM_BURN_BIG, "BURN_L" }, { ANIM_ON_FIRE_SMALL, "ONFIRE_S" }, { ANIM_ON_FIRE_MED, "ONFIRE_M" }, { ANIM_ON_FIRE_BIG, "ONFIRE_L" }, { ANIM_ION_CANNON, "IONSFX" }, { ANIM_ATOM_BLAST, "ATOMSFX" }, { ANIM_CRATE_DEVIATOR, "DEVIATOR" }, { ANIM_CRATE_DOLLAR, "DOLLAR" }, { ANIM_CRATE_EARTH, "EARTH" }, { ANIM_CRATE_EMPULSE, "EMPULSE" }, { ANIM_CRATE_INVUN, "INVUN" }, { ANIM_CRATE_MINE, "MINE" }, { ANIM_CRATE_RAPID, "RAPID" }, { ANIM_CRATE_STEALTH, "STEALTH" }, { ANIM_CRATE_MISSILE, "MISSILE" }, { ANIM_ATOM_DOOR, "ATOMDOOR" }, { ANIM_MOVE_FLASH, "MV_FLASH" }, { ANIM_OILFIELD_BURN, "FLMSPT" }, { ANIM_CHEM_BALL, "CHEMBALL" }, { ANIM_FIRE_SMALL_VIRTUAL, "FIRE_S_V" }, { ANIM_FIRE_MED_VIRTUAL, "FIRE_M_V" }, { ANIM_FIRE_MED2_VIRTUAL, "FIR2_M_V" }, { ANIM_FIRE_TINY_VIRTUAL, "FIRE_T_V" }, { ANIM_BEACON_VIRTUAL, "BEACON_V" }};
static const TwoWayMap<BulletType, std::string> BulletPatchTable = {{ BULLET_SAM, "SAM_BULT" }, { BULLET_CHEMSPRAY, "CHEMSPRA" }, { BULLET_NUKE_UP, "ATOMICUP" }, { BULLET_NUKE_DOWN, "ATOMICDN" }, { BULLET_HONEST_JOHN, "HONESTJB" }, { BULLET_SPREADFIRE, "SPRDFIRE" }, { BULLET_HEADBUTT, "GORE" }, { BULLET_TREXBITE, "CHEW" }};
static const TwoWayMap<InfantryType, std::string> InfantryPatchTable = {{ INFANTRY_E7, "E6" }};
static const TwoWayMap<StructType, std::string> StructPatchTable = {{ STRUCT_GTOWER, "GTWR" }, { STRUCT_ATOWER, "ATWR" }, { STRUCT_OBELISK, "OBLI" }, { STRUCT_RADAR, "HQ" }, { STRUCT_TURRET, "GUN" }, { STRUCT_CONST, "FACT" }, { STRUCT_REFINERY, "PROC" }, { STRUCT_STORAGE, "SILO" }, { STRUCT_HELIPAD, "HPAD" }, { STRUCT_AIRSTRIP, "AFLD" }, { STRUCT_POWER, "NUKE", }, { STRUCT_ADVANCED_POWER, "NUK2" }, { STRUCT_HOSPITAL, "HOSP" }, { STRUCT_BARRACKS, "PYLE" }, { STRUCT_TANKER, "ARCO" }, { STRUCT_REPAIR, "FIX" }, { STRUCT_BIO_LAB, "BIO" }, { STRUCT_TEMPLE, "TMPL" }, { STRUCT_MISSION, "MISS" }, { STRUCT_PUMP, "V19" }, { STRUCT_SANDBAG_WALL, "SBAG" }, { STRUCT_CYCLONE_WALL, "CYCL" }, { STRUCT_BRICK_WALL, "BRIK" }, { STRUCT_BARBWIRE_WALL, "BARB" }, { STRUCT_WOOD_WALL, "WOOD" }};
static const TwoWayMap<UnitType, std::string> UnitPatchTable = {{ UNIT_HTANK, "HTNK" }, { UNIT_MTANK, "MTNK" }, { UNIT_LTANK, "LTNK" }, { UNIT_STANK, "STNK" }, { UNIT_FTANK, "FTNK" }, { UNIT_MLRS, "MSAM" }, { UNIT_BUGGY, "BGGY" }, { UNIT_HARVESTER, "HARV" }, { UNIT_MSAM, "MLRS" }, { UNIT_HOVER, "LST" }, { UNIT_GUNBOAT, "BOAT" }};
static const TwoWayMap<WarheadType, std::string> WarheadPatchTable = {{ WARHEAD_HE, "HE_WARHEAD" }, { WARHEAD_LASER, "LASER_WARHEAD" }};
static const TwoWayMap<WeaponType, std::string> WeaponPatchTable = {{ WEAPON_GRENADE, "GRENADE_WEAPON" }, { WEAPON_MLRS, "WEAPON_MLRS" }, { WEAPON_NAPALM, "NAPALM_WEAPON" }, { WEAPON_STEG, "STEG_WEAPON" }, { WEAPON_TREX, "TREX_WEAPON" }};

/**
 * Internal values that should not be used in INI or Lua APIs.
 * (only required when enum doesn't follow standard X_FIRST/X_LAST convention)
 */
static const std::vector ScenarioVarExcludes = {SCEN_VAR_COUNT};
static const std::vector VocExcludes = { VOC_FIRST, VOC_COUNT };

const std::map<std::string_view, EnumTypeInfoVariant> TdTypeConverter::EnumTypes = {
    //               [Typename]       [Prefix]        [Min Valid Val]    [Max Valid Val]             [INI Patch Table]   [Excluded Vals]
    ENUM_TYPE_PAIR(ArmorType,       "ARMOR_",       ARMOR_NONE,        ARMOR_LAST,                 {},                 {}),
    ENUM_TYPE_PAIR(MPHType,         "MPH_",         MPH_IMMOBILE,      MPH_LIGHT_SPEED,            {},                 {}),
    ENUM_TYPE_PAIR(WeaponType,      "WEAPON_",      WEAPON_NONE,       WEAPON_LAST,                WeaponPatchTable,   {}),
    ENUM_TYPE_PAIR(HousesType,      "HOUSE_",       HOUSE_NONE,        HOUSE_LAST,                 {},                 {}),
    ENUM_TYPE_PAIR(StructType,      "STRUCT_",      STRUCT_NONE,       STRUCT_LAST,                StructPatchTable,   {}),
    ENUM_TYPE_PAIR(FactoryType,     "FACTORY_",     FACTORY_TYPE_NONE, FACTORY_TYPE_BUILDING,      {},                 {}),
    ENUM_TYPE_PAIR(DirType,         "DIR_",         DIR_MIN,           DIR_MAX,                    {},                 {}),
    ENUM_TYPE_PAIR(BSizeType,       "BSIZE_",       BSIZE_NONE,        BSIZE_LAST,                 {},                 {}),
    ENUM_TYPE_PAIR(AircraftType,    "AIRCRAFT_",    AIRCRAFT_NONE,     AIRCRAFT_LAST,              AircraftPatchTable, {}),
    ENUM_TYPE_PAIR(MissionType,     "MISSION_",     MISSION_NONE,      MISSION_LAST,               {},                 {}),
    ENUM_TYPE_PAIR(AnimType,        "ANIM_",        ANIM_NONE,         ANIM_LAST,                  AnimPatchTable,     {}),
    ENUM_TYPE_PAIR(InfantryType,    "INFANTRY_",    INFANTRY_NONE,     INFANTRY_LAST,              InfantryPatchTable, {}),
    ENUM_TYPE_PAIR(UnitType,        "UNIT_",        UNIT_NONE,         UNIT_LAST,                  UnitPatchTable,     {}),
    ENUM_TYPE_PAIR(SpeedType,       "SPEED_",       SPEED_NONE,        SPEED_LAST,                 {},                 {}),
    ENUM_TYPE_PAIR(BulletType,      "BULLET_",      BULLET_NONE,       BULLET_LAST,                BulletPatchTable,   {}),
    ENUM_TYPE_PAIR(WarheadType,     "WARHEAD_",     WARHEAD_NONE,      WARHEAD_LAST,               WarheadPatchTable,  {}),
    ENUM_TYPE_PAIR(VocType,         "VOC_",         VOC_NONE,          VOC_BEACON,                 {},                 VocExcludes),
    ENUM_TYPE_PAIR(PlayerColorType, "REMAP_",       REMAP_NONE,        REMAP_LAST,                 {},                 {}),
    ENUM_TYPE_PAIR(HouseColorType,  "HOUSE_COLOR_", HOUSE_COLOR_GOOD,  HOUSE_COLOR_BRIGHT_NEUTRAL, {},                 {}),
    ENUM_TYPE_PAIR(DiffType,        "DIFF_",        DIFF_FIRST,        DIFF_LAST,                  {},                 {}),
    ENUM_TYPE_PAIR(ScenarioDirType, "SCEN_DIR_",    SCEN_DIR_NONE,     SCEN_DIR_LAST,              {},                 {}),
    ENUM_TYPE_PAIR(ScenarioVarType, "SCEN_VAR_",    SCEN_VAR_NONE,     SCEN_VAR_LOSE,              {},                 ScenarioVarExcludes)
};

bool TdTypeConverter::Rule_Requires_Converter(std::string_view type_name, std::string_view rule) {
return (RegisteredRuleTypes.contains(type_name) && RegisteredRuleTypes[type_name].contains(rule))
    || Rule_Requires_Csv_Converter(type_name, rule);
}

bool TdTypeConverter::Rule_Requires_Csv_Converter(std::string_view type_name, std::string_view rule ){
    return RegisteredCsvRuleTypes.contains(type_name) && RegisteredCsvRuleTypes[type_name].contains(rule);
}

ConverterTypeVariant TdTypeConverter::Get_Rule_Variant(std::string_view type_name, std::string_view rule) {
    return RegisteredRuleTypes[type_name][rule];
}

ConverterTypeVariant TdTypeConverter::Get_Csv_Rule_Variant(std::string_view type_name, std::string_view rule) {
    return RegisteredCsvRuleTypes[type_name][rule];
}

void TdTypeConverter::Set_Rule_With_Variant(RuleSection& section, std::string_view rule, std::string value, const ConverterTypeVariant variant) {
    if (std::get_if<ArmorType>(&variant)) {
        section.Set_With_Converter<ArmorType, TdTypeConverter>(rule, value);
    } else if (std::get_if<MPHType>(&variant)) {
        section.Set_With_Converter<MPHType, TdTypeConverter>(rule, value);
    } else if (std::get_if<WeaponType>(&variant)) {
        section.Set_With_Converter<WeaponType, TdTypeConverter>(rule, value);
    } else if (std::get_if<HousesType>(&variant)) {
        section.Set_With_Converter<HousesType, TdTypeConverter>(rule, value);
    } else if (std::get_if<StructType>(&variant)) {
        section.Set_With_Converter<StructType, TdTypeConverter>(rule, value);
    } else if (std::get_if<FactoryType>(&variant)) {
        section.Set_With_Converter<FactoryType, TdTypeConverter>(rule, value);
    } else if (std::get_if<DirType>(&variant)) {
        section.Set_With_Converter<DirType, TdTypeConverter>(rule, value);
    } else if (std::get_if<BSizeType>(&variant)) {
        section.Set_With_Converter<BSizeType, TdTypeConverter>(rule, value);
    } else if (std::get_if<AircraftType>(&variant)) {
        section.Set_With_Converter<AircraftType, TdTypeConverter>(rule, value);
    } else if (std::get_if<MissionType>(&variant)) {
        section.Set_With_Converter<MissionType, TdTypeConverter>(rule, value);
    } else if (std::get_if<AnimType>(&variant)) {
        section.Set_With_Converter<AnimType, TdTypeConverter>(rule, value);
    } else if (std::get_if<InfantryType>(&variant)) {
        section.Set_With_Converter<InfantryType, TdTypeConverter>(rule, value);
    } else if (std::get_if<UnitType>(&variant)) {
        section.Set_With_Converter<UnitType, TdTypeConverter>(rule, value);
    } else if (std::get_if<SpeedType>(&variant)) {
        section.Set_With_Converter<SpeedType, TdTypeConverter>(rule, value);
    } else if (std::get_if<BulletType>(&variant)) {
        section.Set_With_Converter<BulletType, TdTypeConverter>(rule, value);
    } else if (std::get_if<WarheadType>(&variant)) {
        section.Set_With_Converter<WarheadType, TdTypeConverter>(rule, value);
    } else if (std::get_if<VocType>(&variant)) {
        section.Set_With_Converter<VocType, TdTypeConverter>(rule, value);
    } else if (std::get_if<PlayerColorType>(&variant)) {
        section.Set_With_Converter<PlayerColorType, TdTypeConverter>(rule, value);
    } else if (std::get_if<HouseColorType>(&variant)) {
        section.Set_With_Converter<HouseColorType, TdTypeConverter>(rule, value);
    } else if (std::get_if<DiffType>(&variant)) {
        section.Set_With_Converter<DiffType, TdTypeConverter>(rule, value);
    } else if (std::get_if<ScenarioDirType>(&variant)) {
        section.Set_With_Converter<ScenarioDirType, TdTypeConverter>(rule, value);
    } else if (std::get_if<ScenarioVarType>(&variant)) {
        section.Set_With_Converter<ScenarioVarType, TdTypeConverter>(rule, value);
    } else {
        throw std::invalid_argument("Unsupported ConverterTypeVariant type - this is normally caused by variant being updated without updating supporting code");
    }
}

void TdTypeConverter::Set_Csv_Rule_With_Variant(RuleSection& section, std::string_view rule, std::string csv_value, const ConverterTypeVariant variant) {
    if (std::get_if<ArmorType>(&variant)) {
        section.Set_With_Csv_Converter<ArmorType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<MPHType>(&variant)) {
        section.Set_With_Csv_Converter<MPHType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<WeaponType>(&variant)) {
        section.Set_With_Csv_Converter<WeaponType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<HousesType>(&variant)) {
        section.Set_With_Csv_Converter<HousesType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<StructType>(&variant)) {
        section.Set_With_Csv_Converter<StructType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<FactoryType>(&variant)) {
        section.Set_With_Csv_Converter<FactoryType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<DirType>(&variant)) {
        section.Set_With_Csv_Converter<DirType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<BSizeType>(&variant)) {
        section.Set_With_Csv_Converter<BSizeType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<AircraftType>(&variant)) {
        section.Set_With_Csv_Converter<AircraftType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<MissionType>(&variant)) {
        section.Set_With_Csv_Converter<MissionType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<AnimType>(&variant)) {
        section.Set_With_Csv_Converter<AnimType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<InfantryType>(&variant)) {
        section.Set_With_Csv_Converter<InfantryType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<UnitType>(&variant)) {
        section.Set_With_Csv_Converter<UnitType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<SpeedType>(&variant)) {
        section.Set_With_Csv_Converter<SpeedType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<BulletType>(&variant)) {
        section.Set_With_Csv_Converter<BulletType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<WarheadType>(&variant)) {
        section.Set_With_Csv_Converter<WarheadType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<VocType>(&variant)) {
        section.Set_With_Csv_Converter<VocType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<PlayerColorType>(&variant)) {
        section.Set_With_Csv_Converter<PlayerColorType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<HouseColorType>(&variant)) {
        section.Set_With_Csv_Converter<HouseColorType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<DiffType>(&variant)) {
        section.Set_With_Csv_Converter<DiffType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<ScenarioDirType>(&variant)) {
        section.Set_With_Csv_Converter<ScenarioDirType, TdTypeConverter>(rule, csv_value);
    } else if (std::get_if<ScenarioVarType>(&variant)) {
        section.Set_With_Csv_Converter<ScenarioVarType, TdTypeConverter>(rule, csv_value);
    } else {
        throw std::invalid_argument("Unsupported ConverterTypeVariant type - this is normally caused by variant being updated without updating supporting code");
    }
}

std::string_view TdTypeConverter::Get_Type_Name_Variant(ConverterTypeVariant variant) {
    if (std::get_if<ArmorType>(&variant)) {
        return Get_Type_Name<ArmorType>();
    }
    if (std::get_if<MPHType>(&variant)) {
        return Get_Type_Name<MPHType>();
    }
    if (std::get_if<WeaponType>(&variant)) {
        return Get_Type_Name<WeaponType>();
    }
    if (std::get_if<HousesType>(&variant)) {
        return Get_Type_Name<HousesType>();
    }
    if (std::get_if<StructType>(&variant)) {
        return Get_Type_Name<StructType>();
    }
    if (std::get_if<FactoryType>(&variant)) {
        return Get_Type_Name<FactoryType>();
    }
    if (std::get_if<DirType>(&variant)) {
        return Get_Type_Name<DirType>();
    }
    if (std::get_if<BSizeType>(&variant)) {
        return Get_Type_Name<BSizeType>();
    }
    if (std::get_if<AircraftType>(&variant)) {
        return Get_Type_Name<AircraftType>();
    }
    if (std::get_if<MissionType>(&variant)) {
        return Get_Type_Name<MissionType>();
    }
    if (std::get_if<AnimType>(&variant)) {
        return Get_Type_Name<AnimType>();
    }
    if (std::get_if<InfantryType>(&variant)) {
        return Get_Type_Name<InfantryType>();
    }
    if (std::get_if<UnitType>(&variant)) {
        return Get_Type_Name<UnitType>();
    }
    if (std::get_if<SpeedType>(&variant)) {
        return Get_Type_Name<SpeedType>();
    }
    if (std::get_if<BulletType>(&variant)) {
        return Get_Type_Name<BulletType>();
    }
    if (std::get_if<WarheadType>(&variant)) {
        return Get_Type_Name<WarheadType>();
    }
    if (std::get_if<VocType>(&variant)) {
        return Get_Type_Name<VocType>();
    }
    if (std::get_if<PlayerColorType>(&variant)) {
        return Get_Type_Name<PlayerColorType>();
    }
    if (std::get_if<HouseColorType>(&variant)) {
        return Get_Type_Name<HouseColorType>();
    }
    if (std::get_if<DiffType>(&variant)) {
        return Get_Type_Name<DiffType>();
    }
    if (std::get_if<ScenarioDirType>(&variant)) {
        return Get_Type_Name<ScenarioDirType>();
    }
    if (std::get_if<ScenarioVarType>(&variant)) {
        return Get_Type_Name<ScenarioVarType>();
    }

    throw std::invalid_argument("Unsupported SupportedByTdTypeConverter type - this is normally caused by concept being updated without updating supporting code");
}
