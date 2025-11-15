#include "function.h"
#include "typeconverter.h"

/**
 * Tables to allow mapping between INI strings and enum values, in both directions.
 * (only required for enum values that differ from INI string representation)
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
 * Internal enum values that should not be used in INI or Lua APIs.
 * (only required when enum doesn't follow standard X_FIRST/X_LAST convention or has 'extra' values after X_COUNT)
 */
static const std::vector ScenarioVarExcludes = {SCEN_VAR_COUNT};
static const std::vector VocExcludes = { VOC_FIRST, VOC_COUNT };
static const std::vector SourceExcludes = { SOURCE_FIRST, SOURCE_COUNT };

#define ENUM_TYPE_PAIR(TYPE, ...) { Get_Type_Name<TYPE>(), EnumTypeInfo<TYPE>(__VA_ARGS__) }

// TODO: Determine if LAST/COUNT ETC values are actually excluded from TwoWayMaps (we use these as number boundaries, but don't want the actual enum value X_LAST to parse)
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
    ENUM_TYPE_PAIR(ScenarioVarType, "SCEN_VAR_",    SCEN_VAR_NONE,     SCEN_VAR_LOSE,              {},                 ScenarioVarExcludes),
    ENUM_TYPE_PAIR(SourceType,      "SOURCE_",      SOURCE_NONE,       SOURCE_OCEAN,               {},                 SourceExcludes),
    ENUM_TYPE_PAIR(RadarEnum,       "RADAR_",       RADAR_NONE,        RADAR_OFF,                  {},                 {}),
    ENUM_TYPE_PAIR(RTTIType,        "RTTITYPE_",    RTTI_NONE,         RTTI_LAST,                  {},                 {}),
    ENUM_TYPE_PAIR(ZoneType,        "ZONE_",        ZONE_NONE,         ZONE_LAST,                  {},                 {}),
    ENUM_TYPE_PAIR(StateType,       "STATE_",       STATE_BUILDUP,     STATE_ENDGAME,              {},                 {}),
    ENUM_TYPE_PAIR(VoxType,         "VOX_",         VOX_FIRST,         VOX_LAST,                   {},                 {})
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

#define RULE_VARIANT(TYPE) if (std::get_if<TYPE>(&variant)) { \
    section.Set_With_Converter<TYPE, TdTypeConverter>(rule, value); \
    return; \
}

void TdTypeConverter::Set_Rule_With_Variant(RuleSection& section, std::string_view rule, std::string value, const ConverterTypeVariant variant) {
    RULE_VARIANT(ArmorType)
    RULE_VARIANT(MPHType)
    RULE_VARIANT(WeaponType)
    RULE_VARIANT(HousesType)
    RULE_VARIANT(StructType)
    RULE_VARIANT(FactoryType)
    RULE_VARIANT(DirType)
    RULE_VARIANT(BSizeType)
    RULE_VARIANT(AircraftType)
    RULE_VARIANT(MissionType)
    RULE_VARIANT(AnimType)
    RULE_VARIANT(InfantryType)
    RULE_VARIANT(UnitType)
    RULE_VARIANT(SpeedType)
    RULE_VARIANT(BulletType)
    RULE_VARIANT(WarheadType)
    RULE_VARIANT(VocType)
    RULE_VARIANT(PlayerColorType)
    RULE_VARIANT(HouseColorType)
    RULE_VARIANT(DiffType)
    RULE_VARIANT(ScenarioDirType)
    RULE_VARIANT(ScenarioVarType)
    RULE_VARIANT(SourceType)
    RULE_VARIANT(RadarEnum)
    RULE_VARIANT(RTTIType)
    RULE_VARIANT(ZoneType)
    RULE_VARIANT(StateType)
    RULE_VARIANT(VoxType)

    throw std::invalid_argument("Unsupported ConverterTypeVariant type - this is normally caused by variant being updated without updating supporting code");
}

#define CSV_RULE_VARIANT(TYPE) if (std::get_if<TYPE>(&variant)) { \
    section.Set_With_Csv_Converter<TYPE, TdTypeConverter>(rule, csv_value); \
    return; \
}

void TdTypeConverter::Set_Csv_Rule_With_Variant(RuleSection& section, std::string_view rule, std::string csv_value, const ConverterTypeVariant variant) {
    CSV_RULE_VARIANT(ArmorType)
    CSV_RULE_VARIANT(MPHType)
    CSV_RULE_VARIANT(WeaponType)
    CSV_RULE_VARIANT(HousesType)
    CSV_RULE_VARIANT(StructType)
    CSV_RULE_VARIANT(FactoryType)
    CSV_RULE_VARIANT(DirType)
    CSV_RULE_VARIANT(BSizeType)
    CSV_RULE_VARIANT(AircraftType)
    CSV_RULE_VARIANT(MissionType)
    CSV_RULE_VARIANT(AnimType)
    CSV_RULE_VARIANT(InfantryType)
    CSV_RULE_VARIANT(UnitType)
    CSV_RULE_VARIANT(SpeedType)
    CSV_RULE_VARIANT(BulletType)
    CSV_RULE_VARIANT(WarheadType)
    CSV_RULE_VARIANT(VocType)
    CSV_RULE_VARIANT(PlayerColorType)
    CSV_RULE_VARIANT(HouseColorType)
    CSV_RULE_VARIANT(DiffType)
    CSV_RULE_VARIANT(ScenarioDirType)
    CSV_RULE_VARIANT(ScenarioVarType)
    CSV_RULE_VARIANT(SourceType)
    CSV_RULE_VARIANT(RadarEnum)
    CSV_RULE_VARIANT(RTTIType)
    CSV_RULE_VARIANT(ZoneType)
    CSV_RULE_VARIANT(StateType)
    CSV_RULE_VARIANT(VoxType)

    throw std::invalid_argument("Unsupported ConverterTypeVariant type - this is normally caused by variant being updated without updating supporting code");
}

#define TYPE_NAME_VARIANT(TYPE) if (std::get_if<TYPE>(&variant)) { \
    return Get_Type_Name<TYPE>(); \
}

std::string_view TdTypeConverter::Get_Type_Name_Variant(ConverterTypeVariant variant) {
    TYPE_NAME_VARIANT(ArmorType)
    TYPE_NAME_VARIANT(MPHType)
    TYPE_NAME_VARIANT(WeaponType)
    TYPE_NAME_VARIANT(HousesType)
    TYPE_NAME_VARIANT(StructType)
    TYPE_NAME_VARIANT(FactoryType)
    TYPE_NAME_VARIANT(DirType)
    TYPE_NAME_VARIANT(BSizeType)
    TYPE_NAME_VARIANT(AircraftType)
    TYPE_NAME_VARIANT(MissionType)
    TYPE_NAME_VARIANT(AnimType)
    TYPE_NAME_VARIANT(InfantryType)
    TYPE_NAME_VARIANT(UnitType)
    TYPE_NAME_VARIANT(SpeedType)
    TYPE_NAME_VARIANT(BulletType)
    TYPE_NAME_VARIANT(WarheadType)
    TYPE_NAME_VARIANT(VocType)
    TYPE_NAME_VARIANT(PlayerColorType)
    TYPE_NAME_VARIANT(HouseColorType)
    TYPE_NAME_VARIANT(DiffType)
    TYPE_NAME_VARIANT(ScenarioDirType)
    TYPE_NAME_VARIANT(ScenarioVarType)
    TYPE_NAME_VARIANT(SourceType)
    TYPE_NAME_VARIANT(RadarEnum)
    TYPE_NAME_VARIANT(RTTIType)
    TYPE_NAME_VARIANT(ZoneType)
    TYPE_NAME_VARIANT(StateType)
    TYPE_NAME_VARIANT(VoxType)

    throw std::invalid_argument("Unsupported SupportedByTdTypeConverter type - this is normally caused by concept being updated without updating supporting code");
}
