#include "function.h"

#include "typeconverter.h"

/**
 * Make some type names more human-readable or less confusing.
 */
const std::unordered_map<size_t, std::string_view> TdTypeConverter::TypeNamePatchTable = {
    { TYPE_ID(AnimType), "Animation" },
    { TYPE_ID(HousesType), "House" },
    { TYPE_ID(StructType), "Building" }
};

/**
 * Tables to allow mapping between INI strings and enum values, in both directions.
 * (only required for enum values that differ from INI string representation)
 */
static const TwoWayMap<AircraftType, std::string> AircraftPatchTable = {{ AIRCRAFT_TRANSPORT, "TRAN" }, { AIRCRAFT_HELICOPTER, "HELI" }, { AIRCRAFT_CARGO, "C17" }};
static const TwoWayMap<AnimType, std::string> AnimPatchTable = {{ ANIM_GRENADE, "GRENADEA" }, { ANIM_SMOKE_PUFF, "SMOKEY" }, { ANIM_FIRE_SMALL, "FIRE_S" }, { ANIM_FIRE_MED, "FIRE_M" }, { ANIM_FIRE_MED2, "FIRE_ME" }, { ANIM_FIRE_TINY, "FIRE_T" }, { ANIM_MUZZLE_FLASH, "GUNFIRE" }, { ANIM_BURN_SMALL, "BURN_S" }, { ANIM_BURN_MED, "BURN_M" }, { ANIM_BURN_BIG, "BURN_L" }, { ANIM_ON_FIRE_SMALL, "ONFIRE_S" }, { ANIM_ON_FIRE_MED, "ONFIRE_M" }, { ANIM_ON_FIRE_BIG, "ONFIRE_L" }, { ANIM_ION_CANNON, "IONSFX" }, { ANIM_ATOM_BLAST, "ATOMSFX" }, { ANIM_CRATE_DEVIATOR, "DEVIATOR" }, { ANIM_CRATE_DOLLAR, "DOLLAR" }, { ANIM_CRATE_EARTH, "EARTH" }, { ANIM_CRATE_EMPULSE, "EMPULSE" }, { ANIM_CRATE_INVUN, "INVUN" }, { ANIM_CRATE_MINE, "MINE" }, { ANIM_CRATE_RAPID, "RAPID" }, { ANIM_CRATE_STEALTH, "STEALTH" }, { ANIM_CRATE_MISSILE, "MISSILE" }, { ANIM_ATOM_DOOR, "ATOMDOOR" }, { ANIM_MOVE_FLASH, "MV_FLASH" }, { ANIM_OILFIELD_BURN, "FLMSPT" }, { ANIM_CHEM_BALL, "CHEMBALL" }, { ANIM_FIRE_SMALL_VIRTUAL, "FIRE_S_V" }, { ANIM_FIRE_MED_VIRTUAL, "FIRE_M_V" }, { ANIM_FIRE_MED2_VIRTUAL, "FIR2_M_V" }, { ANIM_FIRE_TINY_VIRTUAL, "FIRE_T_V" }, { ANIM_BEACON_VIRTUAL, "BEACON_V" }};
static const TwoWayMap<BulletType, std::string> BulletPatchTable = {{ BULLET_SAM, "SAM_BULT" }, { BULLET_CHEMSPRAY, "CHEMSPRA" }, { BULLET_NUKE_UP, "ATOMICUP" }, { BULLET_NUKE_DOWN, "ATOMICDN" }, { BULLET_HONEST_JOHN, "HONESTJB" }, { BULLET_SPREADFIRE, "SPRDFIRE" }, { BULLET_HEADBUTT, "GORE" }, { BULLET_TREXBITE, "CHEW" }};
static const TwoWayMap<CCPaletteType, std::string> CCPalettePatchTable = {{ CC_GDI_COLOR, "YELLOW" }, { CC_NOD_COLOR, "RED"}, {CC_BLUE_GREEN, "CYAN"}, {CC_BLUE_GREY, "BLUE"}, {CC_TAN, "BROWN"}};
static const TwoWayMap<HousesType, std::string> HousesPatchTable = {{ HOUSE_GOOD, "GOODGUY" }, { HOUSE_BAD, "BADGUY" }};
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
static const std::vector TemplateExcludes = {TEMPLATE_COUNT};
static const std::vector VocExcludes = {VOC_FIRST, VOC_COUNT};

#define ENUM_TYPE_PAIR(TYPE, ...) { typeid(TYPE).hash_code(), EnumTypeInfo<TYPE>(__VA_ARGS__) }

// Info about each enum supported type, indexed against it's typename
const std::unordered_map<size_t, EnumTypeInfoVariant> TdTypeConverter::EnumTypes = {
    //             [Typename]                    [Prefix]        [Min Valid Val]                         [Max Valid Val]                           [INI Patch Table]    [Excluded Vals]      [Allow non-enum values?]
    ENUM_TYPE_PAIR(AircraftType,                 "AIRCRAFT_",    AIRCRAFT_NONE,                          AIRCRAFT_LAST,                            AircraftPatchTable,  {},                  false),
    ENUM_TYPE_PAIR(AnimType,                     "ANIM_",        ANIM_NONE,                              ANIM_LAST,                                AnimPatchTable,      {},                  false),
    ENUM_TYPE_PAIR(ArmorType,                    "ARMOR_",       ARMOR_NONE,                             ARMOR_LAST,                               {},                  {},                  false),
    ENUM_TYPE_PAIR(BulletType,                   "BULLET_",      BULLET_NONE,                            BULLET_LAST,                              BulletPatchTable,    {},                  false),
    ENUM_TYPE_PAIR(BSizeType,                    "BSIZE_",       BSIZE_NONE,                             BSIZE_LAST,                               {},                  {},                  false),
    ENUM_TYPE_PAIR(BStateType,                   "BSTATE_",      BSTATE_NONE,                            BSTATE_AUX2,                              {},                  {},                  false),
    ENUM_TYPE_PAIR(CCPaletteType,                "CC_",          static_cast<CCPaletteType>(TBLACK),     static_cast<CCPaletteType>(WHITE),        CCPalettePatchTable, {},                  false),
    ENUM_TYPE_PAIR(CloakType,                    "",             UNCLOAKED,                              UNCLOAKING,                               {},                  {},                  false),
    ENUM_TYPE_PAIR(DiffType,                     "DIFF_",        DIFF_NONE,                              DIFF_LAST,                                {},                  {},                  false),
    ENUM_TYPE_PAIR(DirType,                      "DIR_",         DIR_MIN,                                DIR_MAX,                                  {},                  {},                  true),
    ENUM_TYPE_PAIR(DoorClass::DoorStateType,     "IS_",          DoorClass::IS_CLOSED,                   DoorClass::IS_CLOSING,                    {},                  {},                  false),
    ENUM_TYPE_PAIR(DoType,                       "DO_",          DO_NOTHING,                             DO_PLEAD_DEATH,                           {},                  {},                  false),
    ENUM_TYPE_PAIR(EventType,                    "EVENT_",       EVENT_NONE,                             EVENT_LAST,                               {},                  {},                  false),
    ENUM_TYPE_PAIR(FacingType,                   "FACING_",      FACING_NONE,                            FACING_LAST,                              {},                  {},                  false),
    ENUM_TYPE_PAIR(FactoryType,                  "FACTORY_",     FACTORY_TYPE_NONE,                      FACTORY_TYPE_BUILDING,                    {},                  {},                  false),
    ENUM_TYPE_PAIR(GameType,                     "GAME_",        GAME_NORMAL,                            GAME_GLYPHX_MULTIPLAYER,                  {},                  {},                  false),
    ENUM_TYPE_PAIR(HouseColorType,               "HOUSE_COLOR_", HOUSE_COLOR_BAD,                        HOUSE_COLOR_NEUTRAL,                      {},                  {},                  false),
    ENUM_TYPE_PAIR(HousesType,                   "HOUSE_",       HOUSE_NONE,                             HOUSE_LAST,                               HousesPatchTable,    {},                  false),
    ENUM_TYPE_PAIR(InfantryType,                 "INFANTRY_",    INFANTRY_NONE,                          INFANTRY_LAST,                            InfantryPatchTable,  {},                  false),
    ENUM_TYPE_PAIR(KeyNumType,                   "KN_",          KN_NONE,                                KN_MOUSEWHEEL_DOWN,                       {},                  {},                  false),
    ENUM_TYPE_PAIR(KindType,                     "KIND_",        KIND_NONE,                              KIND_TEAMTYPE,                            {},                  {},                  false),
    ENUM_TYPE_PAIR(LandType,                     "LAND_",        LAND_CLEAR,                             LAND_COUNT,                               {},                  {},                  false),
    ENUM_TYPE_PAIR(LayerType,                    "LAYER_",       LAYER_NONE,                             LAYER_LAST,                               {},                  {},                  false),
    ENUM_TYPE_PAIR(MissionType,                  "MISSION_",     MISSION_NONE,                           MISSION_LAST,                             {},                  {},                  false),
    ENUM_TYPE_PAIR(MouseType,                    "MOUSE_",       MOUSE_NORMAL,                           MOUSE_AREA_GUARD,                         {},                  {},                  false),
    ENUM_TYPE_PAIR(MPHType,                      "MPH_",         MPH_IMMOBILE,                           MPH_LIGHT_SPEED,                          {},                  {},                  true),
    ENUM_TYPE_PAIR(OverlayType,                  "OVERLAY_",     OVERLAY_NONE,                           OVERLAY_LAST,                             {},                  {},                  false),
    ENUM_TYPE_PAIR(PlayerColorType,              "REMAP_",       REMAP_NONE,                             REMAP_LAST,                               {},                  {},                  false),
    ENUM_TYPE_PAIR(RadarEnum,                    "RADAR_",       RADAR_NONE,                             RADAR_ON,                                 {},                  {},                  false),
    ENUM_TYPE_PAIR(RadioMessageType,             "RADIO_",       RADIO_STATIC,                           RADIO_ON_DEPOT,                           {},                  {},                  false),
    ENUM_TYPE_PAIR(RTTIType,                     "RTTI_",        RTTI_NONE,                              RTTI_LAST,                                {},                  {},                  false),
    ENUM_TYPE_PAIR(ScenarioDirType,              "SCEN_DIR_",    SCEN_DIR_NONE,                          SCEN_DIR_LAST,                            {},                  {},                  false),
    ENUM_TYPE_PAIR(ScenarioPlayerType,           "SCEN_PLAYER_", SCEN_PLAYER_NONE,                       SCEN_PLAYER_LAST,                         {},                  {},                  false),
    ENUM_TYPE_PAIR(ScenarioVarType,              "SCEN_VAR_",    SCEN_VAR_NONE,                          SCEN_VAR_LOSE,                            {},                  ScenarioVarExcludes, false),
    ENUM_TYPE_PAIR(SpecialWeaponType,            "SPC_",         SPC_NONE,                               SPC_AIR_STRIKE,                           {},                  {},                  false),
    ENUM_TYPE_PAIR(SmudgeType,                   "SMUDGE_",      SMUDGE_NONE,                            SMUDGE_LAST,                              {},                  {},                  false),
    ENUM_TYPE_PAIR(SourceType,                   "SOURCE_",      SOURCE_NONE,                            SOURCE_OCEAN,                             {},                  {},                  false),
    ENUM_TYPE_PAIR(SpeedType,                    "SPEED_",       SPEED_NONE,                             SPEED_LAST,                               {},                  {},                  false),
    ENUM_TYPE_PAIR(StateType,                    "STATE_",       STATE_BUILDUP,                          STATE_ENDGAME,                            {},                  {},                  false),
    ENUM_TYPE_PAIR(StructType,                   "STRUCT_",      STRUCT_NONE,                            STRUCT_LAST,                              StructPatchTable,    {},                  false),
    ENUM_TYPE_PAIR(TeamMissionType,              "TMISSION_",    TMISSION_NONE,                          TMISSION_LAST,                            {},                  {},                  false),
    ENUM_TYPE_PAIR(TemplateType,                 "TEMPLATE_",    TEMPLATE_FIRST,                         TEMPLATE_NONE,                            {},                  TemplateExcludes,    false),
    ENUM_TYPE_PAIR(TerrainType,                  "TERRAIN_",     TERRAIN_NONE,                           TERRAIN_LAST,                             {},                  {},                  false),
    ENUM_TYPE_PAIR(TheaterType,                  "THEATER_",     THEATER_NONE,                           THEATER_LAST,                             {},                  {},                  false),
    ENUM_TYPE_PAIR(TriggerClass::ActionType,     "ACTION_",      TriggerClass::ActionType::ACTION_NONE,  TriggerClass::ActionType::ACTION_LAST,    {},                  {},                  false),
    ENUM_TYPE_PAIR(TriggerClass::PersistantType, "",             TriggerClass::PersistantType::VOLATILE, TriggerClass::PersistantType::PERSISTANT, {},                  {},                  false),
    ENUM_TYPE_PAIR(UnitType,                     "UNIT_",        UNIT_NONE,                              UNIT_LAST,                                UnitPatchTable,      {},                  false),
    ENUM_TYPE_PAIR(UrgencyType,                  "URGENCY_",     URGENCY_NONE,                           URGENCY_FIRST,                            {},                  {},                  false),
    ENUM_TYPE_PAIR(VocType,                      "VOC_",         VOC_NONE,                               VOC_BEACON,                               {},                  VocExcludes,         false),
    ENUM_TYPE_PAIR(VoxType,                      "VOX_",         VOX_NONE,                               VOX_LAST,                                 {},                  {},                  false),
    ENUM_TYPE_PAIR(WarheadType,                  "WARHEAD_",     WARHEAD_NONE,                           WARHEAD_LAST,                             WarheadPatchTable,   {},                  false),
    ENUM_TYPE_PAIR(WeaponType,                   "WEAPON_",      WEAPON_NONE,                            WEAPON_LAST,                              WeaponPatchTable,    {},                  false),
    ENUM_TYPE_PAIR(ZoneType,                     "ZONE_",        ZONE_NONE,                              ZONE_LAST,                                {},                  {},                  false)
};

void TdTypeConverter::Reset_Rule_Type_Registry()
{
    RegisteredRuleTypes.clear();
    RegisteredCsvRuleTypes.clear();
}

bool TdTypeConverter::Rule_Requires_Converter(
    const std::string_view& type_name,
    const std::string_view& rule
)
{
    return (RegisteredRuleTypes.contains(type_name.data())
            && RegisteredRuleTypes[type_name.data()].contains(rule.data()))
        || Rule_Requires_Csv_Converter(type_name, rule);
}

bool TdTypeConverter::Rule_Requires_Converter(
    const RuleSection& section,
    const std::string_view& rule
)
{
    const auto& type_name = section.Get_Converter_Section_Type_Name();

    return type_name.has_value() && Rule_Requires_Converter(type_name->data(), rule);
}

bool TdTypeConverter::Rule_Requires_Csv_Converter(
    const std::string_view& type_name,
    const std::string_view& rule
)
{
    return RegisteredCsvRuleTypes.contains(type_name.data())
        && RegisteredCsvRuleTypes[type_name.data()].contains(rule.data());
}

bool TdTypeConverter::Rule_Requires_Csv_Converter(
    const RuleSection& section,
    const std::string_view& rule
)
{
    const auto& type_name = section.Get_Converter_Section_Type_Name();

    return type_name.has_value() && Rule_Requires_Csv_Converter(type_name->data(), rule);
}

ConverterTypeVariant TdTypeConverter::Get_Rule_Variant(
    const std::string_view& type_name,
    const std::string_view& rule
)
{
    return RegisteredRuleTypes[type_name.data()][rule.data()];
}

ConverterTypeVariant TdTypeConverter::Get_Csv_Rule_Variant(
    const std::string_view& type_name,
    const std::string_view& rule
)
{
    return RegisteredCsvRuleTypes[type_name.data()][rule.data()];
}

void TdTypeConverter::Set_Rule_With_Variant(
    RuleSection& section,
    const std::string_view& rule,
    const std::string& value,
    const ConverterTypeVariant& variant
)
{
    std::visit([&](const auto& t) {
        using T = std::decay_t<decltype(t)>;

        section.Set_With_Converter<T, TdTypeConverter>(rule, value);
    }, variant);
}

void TdTypeConverter::Set_Csv_Rule_With_Variant(
    RuleSection& section,
    const std::string_view& rule,
    const std::string& csv_value,
    const ConverterTypeVariant& variant
)
{
    std::visit([&](const auto& t) {
        using T = std::decay_t<decltype(t)>;

        section.Set_With_Csv_Converter<T, TdTypeConverter>(rule, csv_value);
    }, variant);
}

std::string_view TdTypeConverter::Get_Type_Name_Variant(const ConverterTypeVariant& variant)
{
    return std::visit([&](const auto& t) {
        using T = std::decay_t<decltype(t)>;

        return Get_Type_Name<T>();
    }, variant);
}

std::string TdTypeConverter::To_String_Variant(const ConverterTypeVariant& variant)
{
    return std::visit([&](const auto& t) {
        using T = std::decay_t<decltype(t)>;

        return To_String<T>(std::get<T>(variant));
    }, variant);
}

std::vector<std::string> TdTypeConverter::Get_Valid_Strings_Variant(const ConverterTypeVariant& variant)
{
    return std::visit([&](const auto& t) {
        using T = std::decay_t<decltype(t)>;
        return Get_Valid_Strings<T>();
    }, variant);
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
        RTTI_TYPE_TO_STRING(RTTI_SPECIAL, SpecialWeaponType, instance_id)

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
        TRY_PARSE_RTTI_TYPE(RTTI_SPECIAL, SpecialWeaponType, instance)

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
