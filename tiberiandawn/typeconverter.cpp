#include "function.h"
#include "typeconverter.h"

/**
 * Make some type names more human-readable or less confusing.
 */
const std::map<std::string, std::string_view> TdTypeConverter::TypeNamePatchTable = {
    { NAMEOF(AnimType), "Animation" },
    { NAMEOF(StructType), "Building" },
    { NAMEOF(HousesType), "House" }
};

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
static const TwoWayMap<HousesType, std::string> HousesPatchTable = {{ HOUSE_GOOD, "GOODGUY" }, { HOUSE_BAD, "BADGUY" }};
static const TwoWayMap<CCPaletteType, std::string> CCPalettePatchTable = {{ CC_GDI_COLOR, "YELLOW" }, { CC_NOD_COLOR, "RED"}, {CC_BLUE_GREEN, "CYAN"}, {CC_BLUE_GREY, "BLUE"}, {CC_TAN, "BROWN"}};

/**
 * Internal enum values that should not be used in INI or Lua APIs.
 * (only required when enum doesn't follow standard X_FIRST/X_LAST convention or has 'extra' values after X_COUNT)
 */
static const std::vector ScenarioVarExcludes = {SCEN_VAR_COUNT};
static const std::vector VocExcludes = {VOC_FIRST, VOC_COUNT};
static const std::vector TemplateExcludes = {TEMPLATE_COUNT};

#define ENUM_TYPE_PAIR(TYPE, ...) { Get_Type_Name<TYPE>(), EnumTypeInfo<TYPE>(__VA_ARGS__) }

// Info about each enum supported type, indexed against it's typename
const std::map<std::string_view, EnumTypeInfoVariant> TdTypeConverter::EnumTypes = {
    //            [Typename]                     [Prefix]        [Min Valid Val]                         [Max Valid Val]                           [INI Patch Table]    [Excluded Vals]      [Allow non-enum values?]
    ENUM_TYPE_PAIR(ArmorType,                    "ARMOR_",       ARMOR_NONE,                             ARMOR_LAST,                               {},                  {},                  false),
    ENUM_TYPE_PAIR(MPHType,                      "MPH_",         MPH_IMMOBILE,                           MPH_LIGHT_SPEED,                          {},                  {},                  true),
    ENUM_TYPE_PAIR(WeaponType,                   "WEAPON_",      WEAPON_NONE,                            WEAPON_LAST,                              WeaponPatchTable,    {},                  false),
    ENUM_TYPE_PAIR(HousesType,                   "HOUSE_",       HOUSE_NONE,                             HOUSE_LAST,                               HousesPatchTable,    {},                  false),
    ENUM_TYPE_PAIR(StructType,                   "STRUCT_",      STRUCT_NONE,                            STRUCT_LAST,                              StructPatchTable,    {},                  false),
    ENUM_TYPE_PAIR(FactoryType,                  "FACTORY_",     FACTORY_TYPE_NONE,                      FACTORY_TYPE_BUILDING,                    {},                  {},                  false),
    ENUM_TYPE_PAIR(DirType,                      "DIR_",         DIR_MIN,                                DIR_MAX,                                  {},                  {},                  true),
    ENUM_TYPE_PAIR(BSizeType,                    "BSIZE_",       BSIZE_NONE,                             BSIZE_LAST,                               {},                  {},                  false),
    ENUM_TYPE_PAIR(AircraftType,                 "AIRCRAFT_",    AIRCRAFT_NONE,                          AIRCRAFT_LAST,                            AircraftPatchTable,  {},                  false),
    ENUM_TYPE_PAIR(MissionType,                  "MISSION_",     MISSION_NONE,                           MISSION_LAST,                             {},                  {},                  false),
    ENUM_TYPE_PAIR(AnimType,                     "ANIM_",        ANIM_NONE,                              ANIM_LAST,                                AnimPatchTable,      {},                  false),
    ENUM_TYPE_PAIR(InfantryType,                 "INFANTRY_",    INFANTRY_NONE,                          INFANTRY_LAST,                            InfantryPatchTable,  {},                  false),
    ENUM_TYPE_PAIR(UnitType,                     "UNIT_",        UNIT_NONE,                              UNIT_LAST,                                UnitPatchTable,      {},                  false),
    ENUM_TYPE_PAIR(SpeedType,                    "SPEED_",       SPEED_NONE,                             SPEED_LAST,                               {},                  {},                  false),
    ENUM_TYPE_PAIR(BulletType,                   "BULLET_",      BULLET_NONE,                            BULLET_LAST,                              BulletPatchTable,    {},                  false),
    ENUM_TYPE_PAIR(WarheadType,                  "WARHEAD_",     WARHEAD_NONE,                           WARHEAD_LAST,                             WarheadPatchTable,   {},                  false),
    ENUM_TYPE_PAIR(VocType,                      "VOC_",         VOC_NONE,                               VOC_BEACON,                               {},                  VocExcludes,         false),
    ENUM_TYPE_PAIR(PlayerColorType,              "REMAP_",       REMAP_NONE,                             REMAP_LAST,                               {},                  {},                  false),
    ENUM_TYPE_PAIR(HouseColorType,               "HOUSE_COLOR_", HOUSE_COLOR_BAD,                        HOUSE_COLOR_NEUTRAL,                      {},                  {},                  false),
    ENUM_TYPE_PAIR(DiffType,                     "DIFF_",        DIFF_FIRST,                             DIFF_LAST,                                {},                  {},                  false),
    ENUM_TYPE_PAIR(ScenarioDirType,              "SCEN_DIR_",    SCEN_DIR_NONE,                          SCEN_DIR_LAST,                            {},                  {},                  false),
    ENUM_TYPE_PAIR(ScenarioVarType,              "SCEN_VAR_",    SCEN_VAR_NONE,                          SCEN_VAR_LOSE,                            {},                  ScenarioVarExcludes, false),
    ENUM_TYPE_PAIR(SourceType,                   "SOURCE_",      SOURCE_NONE,                            SOURCE_OCEAN,                             {},                  {},                  false),
    ENUM_TYPE_PAIR(RadarEnum,                    "RADAR_",       RADAR_NONE,                             RADAR_OFF,                                {},                  {},                  false),
    ENUM_TYPE_PAIR(RTTIType,                     "RTTI_",        RTTI_NONE,                              RTTI_LAST,                                {},                  {},                  false),
    ENUM_TYPE_PAIR(ZoneType,                     "ZONE_",        ZONE_NONE,                              ZONE_LAST,                                {},                  {},                  false),
    ENUM_TYPE_PAIR(StateType,                    "STATE_",       STATE_BUILDUP,                          STATE_ENDGAME,                            {},                  {},                  false),
    ENUM_TYPE_PAIR(VoxType,                      "VOX_",         VOX_NONE,                               VOX_LAST,                                 {},                  {},                  false),
    ENUM_TYPE_PAIR(MouseType,                    "MOUSE_",       MOUSE_NORMAL,                           MOUSE_AREA_GUARD,                         {},                  {},                  false),
    ENUM_TYPE_PAIR(TheaterType,                  "THEATER_",     THEATER_NONE,                           THEATER_LAST,                             {},                  {},                  false),
    ENUM_TYPE_PAIR(TemplateType,                 "TEMPLATE_",    TEMPLATE_FIRST,                         TEMPLATE_NONE,                            {},                  TemplateExcludes,    false),
    ENUM_TYPE_PAIR(OverlayType,                  "OVERLAY_",     OVERLAY_NONE,                           OVERLAY_LAST,                             {},                  {},                  false),
    ENUM_TYPE_PAIR(SmudgeType,                   "SMUDGE_",      SMUDGE_NONE,                            SMUDGE_LAST,                              {},                  {},                  false),
    ENUM_TYPE_PAIR(LandType,                     "LAND_",        LAND_CLEAR,                             LAND_COUNT,                               {},                  {},                  false),
    ENUM_TYPE_PAIR(TeamMissionType,              "TMISSION_",    TMISSION_NONE,                          TMISSION_LAST,                            {},                  {},                  false),
    ENUM_TYPE_PAIR(RadioMessageType,             "RADIO_",       RADIO_STATIC,                           RADIO_ON_DEPOT,                           {},                  {},                  false),
    ENUM_TYPE_PAIR(CloakType,                    "",             UNCLOAKED,                              UNCLOAKING,                               {},                  {},                  false),
    ENUM_TYPE_PAIR(FacingType,                   "FACING_",      FACING_NONE,                            FACING_LAST,                              {},                  {},                  false),
    ENUM_TYPE_PAIR(DoorClass::DoorStateType,     "IS_",          DoorClass::IS_CLOSED,                   DoorClass::IS_CLOSING,                    {},                  {},                  false),
    ENUM_TYPE_PAIR(KindType,                     "KIND_",        KIND_NONE,                              KIND_TEAMTYPE,                            {},                  {},                  false),
    ENUM_TYPE_PAIR(DoType,                       "DO_",          DO_NOTHING,                             DO_PLEAD_DEATH,                           {},                  {},                  false),
    ENUM_TYPE_PAIR(BStateType,                   "BSTATE_",      BSTATE_NONE,                            BSTATE_AUX2,                              {},                  {},                  false),
    ENUM_TYPE_PAIR(EventType,                    "EVENT_",       EVENT_NONE,                             EVENT_LAST,                               {},                  {},                  false),
    ENUM_TYPE_PAIR(TriggerClass::ActionType,     "ACTION_",      TriggerClass::ActionType::ACTION_NONE,  TriggerClass::ActionType::ACTION_LAST,    {},                  {},                  false),
    ENUM_TYPE_PAIR(TriggerClass::PersistantType, "",             TriggerClass::PersistantType::VOLATILE, TriggerClass::PersistantType::PERSISTANT, {},                  {},                  false),
    ENUM_TYPE_PAIR(TerrainType,                  "TERRAIN_",     TERRAIN_NONE,                           TERRAIN_LAST,                             {},                  {},                  false),
    ENUM_TYPE_PAIR(ScenarioPlayerType,           "SCEN_PLAYER_", SCEN_PLAYER_NONE,                       SCEN_PLAYER_LAST,                         {},                  {},                  false),
    ENUM_TYPE_PAIR(LayerType,                    "LAYER_",       LAYER_NONE,                             LAYER_LAST,                               {},                  {},                  false),
    ENUM_TYPE_PAIR(UrgencyType,                  "URGENCY_",     URGENCY_NONE,                           URGENCY_FIRST,                            {},                  {},                  false),
    ENUM_TYPE_PAIR(CCPaletteType,                "CC_",          static_cast<CCPaletteType>(TBLACK),     static_cast<CCPaletteType>(WHITE),        CCPalettePatchTable, {},                  false),
    ENUM_TYPE_PAIR(GameType,                     "GAME_",        GAME_NORMAL,                            GAME_GLYPHX_MULTIPLAYER,                  {},                  {},                  false)
};

bool TdTypeConverter::Rule_Requires_Converter(
    const std::string_view& type_name,
    const std::string_view& rule
)
{
    return (RegisteredRuleTypes.contains(type_name) && RegisteredRuleTypes[type_name].contains(rule))
        || Rule_Requires_Csv_Converter(type_name, rule);
}

bool TdTypeConverter::Rule_Requires_Csv_Converter(
    const std::string_view& type_name,
    const std::string_view& rule
)
{
    return RegisteredCsvRuleTypes.contains(type_name) && RegisteredCsvRuleTypes[type_name].contains(rule);
}

ConverterTypeVariant TdTypeConverter::Get_Rule_Variant(
    const std::string_view& type_name,
    const std::string_view& rule
)
{
    return RegisteredRuleTypes[type_name][rule];
}

ConverterTypeVariant TdTypeConverter::Get_Csv_Rule_Variant(
    const std::string_view& type_name,
    const std::string_view& rule
)
{
    return RegisteredCsvRuleTypes[type_name][rule];
}

#define RULE_VARIANT(TYPE) if (std::holds_alternative<TYPE>(variant)) { \
    section.Set_With_Converter<TYPE, TdTypeConverter>(rule, value); \
    return; \
}

void TdTypeConverter::Set_Rule_With_Variant(
    RuleSection& section,
    const std::string_view& rule,
    const std::string& value,
    const ConverterTypeVariant& variant
)
{
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
    RULE_VARIANT(MouseType)
    RULE_VARIANT(TheaterType)
    RULE_VARIANT(TemplateType)
    RULE_VARIANT(OverlayType)
    RULE_VARIANT(SmudgeType)
    RULE_VARIANT(LandType)
    RULE_VARIANT(TeamMissionType)
    RULE_VARIANT(RadioMessageType)
    RULE_VARIANT(CloakType)
    RULE_VARIANT(FacingType)
    RULE_VARIANT(DoorClass::DoorStateType)
    RULE_VARIANT(KindType)
    RULE_VARIANT(DoType)
    RULE_VARIANT(BStateType)
    RULE_VARIANT(EventType)
    RULE_VARIANT(TriggerClass::ActionType)
    RULE_VARIANT(TriggerClass::PersistantType)
    RULE_VARIANT(TerrainType)
    RULE_VARIANT(ScenarioPlayerType)
    RULE_VARIANT(LayerType)
    RULE_VARIANT(UrgencyType)
    RULE_VARIANT(CCPaletteType)
    RULE_VARIANT(GameType)

    throw std::invalid_argument("Unsupported ConverterTypeVariant type - this is normally caused by variant being updated without updating supporting code");
}

#define CSV_RULE_VARIANT(TYPE) if (std::holds_alternative<TYPE>(variant)) { \
    section.Set_With_Csv_Converter<TYPE, TdTypeConverter>(rule, csv_value); \
    return; \
}

void TdTypeConverter::Set_Csv_Rule_With_Variant(
    RuleSection& section,
    const std::string_view& rule,
    const std::string& csv_value,
    const ConverterTypeVariant& variant
)
{
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
    CSV_RULE_VARIANT(MouseType)
    CSV_RULE_VARIANT(TheaterType)
    CSV_RULE_VARIANT(TemplateType)
    CSV_RULE_VARIANT(OverlayType)
    CSV_RULE_VARIANT(SmudgeType)
    CSV_RULE_VARIANT(LandType)
    CSV_RULE_VARIANT(TeamMissionType)
    CSV_RULE_VARIANT(RadioMessageType)
    CSV_RULE_VARIANT(CloakType)
    CSV_RULE_VARIANT(FacingType)
    CSV_RULE_VARIANT(DoorClass::DoorStateType)
    CSV_RULE_VARIANT(KindType)
    CSV_RULE_VARIANT(DoType)
    CSV_RULE_VARIANT(BStateType)
    CSV_RULE_VARIANT(EventType)
    CSV_RULE_VARIANT(TriggerClass::ActionType)
    CSV_RULE_VARIANT(TriggerClass::PersistantType)
    CSV_RULE_VARIANT(TerrainType)
    CSV_RULE_VARIANT(ScenarioPlayerType)
    CSV_RULE_VARIANT(LayerType)
    CSV_RULE_VARIANT(UrgencyType)
    CSV_RULE_VARIANT(CCPaletteType)
    CSV_RULE_VARIANT(GameType)

    throw std::invalid_argument("Unsupported ConverterTypeVariant type - this is normally caused by variant being updated without updating supporting code");
}

#define TYPE_NAME_VARIANT(TYPE) if (std::holds_alternative<TYPE>(variant)) { \
    return Get_Type_Name<TYPE>(); \
}

std::string_view TdTypeConverter::Get_Type_Name_Variant(const ConverterTypeVariant& variant) {
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
    TYPE_NAME_VARIANT(MouseType)
    TYPE_NAME_VARIANT(TheaterType)
    TYPE_NAME_VARIANT(TemplateType)
    TYPE_NAME_VARIANT(OverlayType)
    TYPE_NAME_VARIANT(SmudgeType)
    TYPE_NAME_VARIANT(LandType)
    TYPE_NAME_VARIANT(TeamMissionType)
    TYPE_NAME_VARIANT(RadioMessageType)
    TYPE_NAME_VARIANT(CloakType)
    TYPE_NAME_VARIANT(FacingType)
    TYPE_NAME_VARIANT(DoorClass::DoorStateType)
    TYPE_NAME_VARIANT(KindType)
    TYPE_NAME_VARIANT(DoType)
    TYPE_NAME_VARIANT(BStateType)
    TYPE_NAME_VARIANT(EventType)
    TYPE_NAME_VARIANT(TriggerClass::ActionType)
    TYPE_NAME_VARIANT(TriggerClass::PersistantType)
    TYPE_NAME_VARIANT(TerrainType)
    TYPE_NAME_VARIANT(ScenarioPlayerType)
    TYPE_NAME_VARIANT(LayerType)
    TYPE_NAME_VARIANT(UrgencyType)
    TYPE_NAME_VARIANT(CCPaletteType)
    TYPE_NAME_VARIANT(GameType)

    throw std::invalid_argument("Unsupported ConverterTypeVariant type - this is normally caused by variant being updated without updating supporting code");
}

#define TO_STRING_VARIANT(TYPE) if (std::holds_alternative<TYPE>(variant)) { \
    return To_String<TYPE>(std::get<TYPE>(variant)); \
}

std::string TdTypeConverter::To_String_Variant(const ConverterTypeVariant& variant)
{
    TO_STRING_VARIANT(ArmorType)
    TO_STRING_VARIANT(MPHType)
    TO_STRING_VARIANT(WeaponType)
    TO_STRING_VARIANT(HousesType)
    TO_STRING_VARIANT(StructType)
    TO_STRING_VARIANT(FactoryType)
    TO_STRING_VARIANT(DirType)
    TO_STRING_VARIANT(BSizeType)
    TO_STRING_VARIANT(AircraftType)
    TO_STRING_VARIANT(MissionType)
    TO_STRING_VARIANT(AnimType)
    TO_STRING_VARIANT(InfantryType)
    TO_STRING_VARIANT(UnitType)
    TO_STRING_VARIANT(SpeedType)
    TO_STRING_VARIANT(BulletType)
    TO_STRING_VARIANT(WarheadType)
    TO_STRING_VARIANT(VocType)
    TO_STRING_VARIANT(PlayerColorType)
    TO_STRING_VARIANT(HouseColorType)
    TO_STRING_VARIANT(DiffType)
    TO_STRING_VARIANT(ScenarioDirType)
    TO_STRING_VARIANT(ScenarioVarType)
    TO_STRING_VARIANT(SourceType)
    TO_STRING_VARIANT(RadarEnum)
    TO_STRING_VARIANT(RTTIType)
    TO_STRING_VARIANT(ZoneType)
    TO_STRING_VARIANT(StateType)
    TO_STRING_VARIANT(VoxType)
    TO_STRING_VARIANT(MouseType)
    TO_STRING_VARIANT(TheaterType)
    TO_STRING_VARIANT(TemplateType)
    TO_STRING_VARIANT(OverlayType)
    TO_STRING_VARIANT(SmudgeType)
    TO_STRING_VARIANT(LandType)
    TO_STRING_VARIANT(TeamMissionType)
    TO_STRING_VARIANT(RadioMessageType)
    TO_STRING_VARIANT(CloakType)
    TO_STRING_VARIANT(FacingType)
    TO_STRING_VARIANT(DoorClass::DoorStateType)
    TO_STRING_VARIANT(KindType)
    TO_STRING_VARIANT(DoType)
    TO_STRING_VARIANT(BStateType)
    TO_STRING_VARIANT(EventType)
    TO_STRING_VARIANT(TriggerClass::ActionType)
    TO_STRING_VARIANT(TriggerClass::PersistantType)
    TO_STRING_VARIANT(TerrainType)
    TO_STRING_VARIANT(ScenarioPlayerType)
    TO_STRING_VARIANT(LayerType)
    TO_STRING_VARIANT(UrgencyType)
    TO_STRING_VARIANT(CCPaletteType)
    TO_STRING_VARIANT(GameType)

    throw std::invalid_argument("Unsupported ConverterTypeVariant type - this is normally caused by variant being updated without updating supporting code");
}

#define RTTI_TYPE_TO_STRING(RTTI, TYPE, ID) case RTTI: \
    return To_String<TYPE>(static_cast<TYPE>(ID));

std::optional<std::string> TdTypeConverter::RTTI_Instance_To_String(const RTTIType& type, const int& instance_id)
{
    switch (type) {
    case RTTI_NONE:
            return std::nullopt;

        RTTI_TYPE_TO_STRING(RTTI_INFANTRYTYPE, InfantryType, instance_id)
        RTTI_TYPE_TO_STRING(RTTI_UNITTYPE, UnitType, instance_id)
        RTTI_TYPE_TO_STRING(RTTI_AIRCRAFTTYPE, AircraftType, instance_id)
        RTTI_TYPE_TO_STRING(RTTI_BUILDINGTYPE, StructType, instance_id)
        RTTI_TYPE_TO_STRING(RTTI_ANIMTYPE, AnimType, instance_id)
        RTTI_TYPE_TO_STRING(RTTI_BULLETTYPE, BulletType, instance_id)
        RTTI_TYPE_TO_STRING(RTTI_OVERLAYTYPE, OverlayType, instance_id)
        RTTI_TYPE_TO_STRING(RTTI_SMUDGETYPE, SmudgeType, instance_id)
        RTTI_TYPE_TO_STRING(RTTI_TEMPLATETYPE, TemplateType, instance_id)
        RTTI_TYPE_TO_STRING(RTTI_TERRAINTYPE, TerrainType, instance_id)

        default:
            throw std::invalid_argument(
                std::format("Unsupported RTTI type passed: {}", To_String(type))
            );
    }
}

#define TRY_PARSE_RTTI_TYPE(RTTI, TYPE, ID) case RTTI: \
    { \
        const auto& TYPE##_result = Try_Parse<TYPE>(ID); \
\
        if (!TYPE##_result.has_value()) { \
          return std::nullopt; \
        } \
\
        return static_cast<int>(*TYPE##_result); \
    }

std::optional<int> TdTypeConverter::Try_Parse_RTTI_Instance(const RTTIType& type, const std::string& instance)
{
    switch (type) {
        case RTTI_NONE:
            return 0;

        TRY_PARSE_RTTI_TYPE(RTTI_INFANTRYTYPE, InfantryType, instance)
        TRY_PARSE_RTTI_TYPE(RTTI_UNITTYPE, UnitType, instance)
        TRY_PARSE_RTTI_TYPE(RTTI_AIRCRAFTTYPE, AircraftType, instance)
        TRY_PARSE_RTTI_TYPE(RTTI_BUILDINGTYPE, StructType, instance)
        TRY_PARSE_RTTI_TYPE(RTTI_ANIMTYPE, AnimType, instance)
        TRY_PARSE_RTTI_TYPE(RTTI_BULLETTYPE, BulletType, instance)
        TRY_PARSE_RTTI_TYPE(RTTI_OVERLAYTYPE, OverlayType, instance)
        TRY_PARSE_RTTI_TYPE(RTTI_SMUDGETYPE, SmudgeType, instance)
        TRY_PARSE_RTTI_TYPE(RTTI_TEMPLATETYPE, TemplateType, instance)
        TRY_PARSE_RTTI_TYPE(RTTI_TERRAINTYPE, TerrainType, instance)

        default:
            throw std::invalid_argument(
                std::format("Unsupported RTTI type passed: {}", To_String(type))
            );
    }
}

nlohmann::json TdTypeConverter::Object_Target_Array_To_Json(
    const ObjectClass* const* source,
    const unsigned int& length
)
{
    nlohmann::json target = nlohmann::json::array();

    for (auto i = 0; i < length; i++) {
        const auto element = *(source + i);

        target.emplace_back(OBJECT_PTR_TO_TARGET(element));
    }

    return target;
}

#define TYPE_TO_REF_CASE(RTTI, KIND, TYPE) case RTTI: \
    reference.Kind = KIND; \
    reference.Instance = To_String(dynamic_cast<const TYPE*>(source)->Type); \
    break;

nlohmann::json TdTypeConverter::Techno_Type_To_Reference_Json(const ObjectTypeClass* source)
{
    TechnoTypeClassJsonReference reference;

    if (source == nullptr) {
        reference.Kind = KIND_NONE;

        return reference;
    }

    switch (const auto source_type = source->What_Am_I()) {
        TYPE_TO_REF_CASE(RTTI_INFANTRYTYPE, KIND_INFANTRY, InfantryTypeClass)
        TYPE_TO_REF_CASE(RTTI_UNITTYPE, KIND_UNIT, UnitTypeClass)
        TYPE_TO_REF_CASE(RTTI_AIRCRAFTTYPE, KIND_AIRCRAFT, AircraftTypeClass)
        TYPE_TO_REF_CASE(RTTI_BUILDINGTYPE, KIND_BUILDING, BuildingTypeClass)
        TYPE_TO_REF_CASE(RTTI_BULLETTYPE, KIND_BULLET, BulletTypeClass)
        TYPE_TO_REF_CASE(RTTI_ANIMTYPE, KIND_ANIMATION, AnimTypeClass)
        TYPE_TO_REF_CASE(RTTI_TERRAINTYPE, KIND_TERRAIN, TerrainTypeClass)
        TYPE_TO_REF_CASE(RTTI_TEMPLATETYPE, KIND_TEMPLATE, TemplateTypeClass)

        default:
            throw std::invalid_argument(
                std::format("Unsupported TechnoTypeClass type: {}", To_String(source_type))
            );
    }

    return reference;
}

TechnoTypeClassJsonReference TdTypeConverter::Techno_Type_Reference_From_Json(
    const nlohmann::json& source,
    const std::string& json_path
)
{
    CncJsonUtils::Assert_Json_Is_Object_With_Keys(source, json_path, { NAMEOF(Kind) });

    TechnoTypeClassJsonReference ref = source;

    return ref;
}
