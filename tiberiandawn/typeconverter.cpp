#include "function.h"
#include "typeconverter.h"

#define ARMOR_PAIR(ARMOR_NAME) { ARMOR_##ARMOR_NAME, #ARMOR_NAME }
#define MPH_PAIR(MPH_NAME) { MPH_##MPH_NAME, #MPH_NAME }
#define WEAPON_PAIR(WEAPON_NAME) { WEAPON_##WEAPON_NAME, #WEAPON_NAME }
#define HOUSE_PAIR(HOUSE_NAME) { HOUSE_##HOUSE_NAME, #HOUSE_NAME }
#define STRUCT_PAIR(STRUCT_NAME) { STRUCT_##STRUCT_NAME, #STRUCT_NAME }
#define FACTORY_PAIR(FACTORY_NAME) { FACTORY_TYPE_##FACTORY_NAME, #FACTORY_NAME }
#define DIR_PAIR(DIR_NAME) { DIR_##DIR_NAME, #DIR_NAME }
#define BSIZE_PAIR(BSIZE_NAME) { BSIZE_##BSIZE_NAME, #BSIZE_NAME }
#define AIRCRAFT_PAIR(AIRCRAFT_NAME) { AIRCRAFT_##AIRCRAFT_NAME, #AIRCRAFT_NAME }
#define MISSION_PAIR(MISSION_NAME) { MISSION_##MISSION_NAME, #MISSION_NAME }
#define ANIM_PAIR(ANIM_NAME) { ANIM_##ANIM_NAME, #ANIM_NAME }
#define INFANTRY_PAIR(INFANTRY_NAME) { INFANTRY_##INFANTRY_NAME, #INFANTRY_NAME }
#define UNIT_PAIR(UNIT_NAME) { UNIT_##UNIT_NAME, #UNIT_NAME }
#define SPEED_PAIR(SPEED_NAME) { SPEED_##SPEED_NAME, #SPEED_NAME }
#define BULLET_PAIR(BULLET_NAME) { BULLET_##BULLET_NAME, #BULLET_NAME }
#define WARHEAD_PAIR(WARHEAD_NAME) { WARHEAD_##WARHEAD_NAME, #WARHEAD_NAME }
#define VOC_PAIR(VOC_NAME) { VOC_##VOC_NAME, #VOC_NAME }
#define PLAYER_COLOR_PAIR(COLOR_NAME) { REMAP_##COLOR_NAME, #COLOR_NAME }
#define HOUSE_COLOR_PAIR(COLOR_NAME) { HOUSE_COLOR_##COLOR_NAME, #COLOR_NAME }

const TwoWayMap<ArmorType, std::string> TdTypeConverter::Armor_Types = {
    ARMOR_PAIR(NONE),
    ARMOR_PAIR(WOOD),
    ARMOR_PAIR(ALUMINUM),
    ARMOR_PAIR(STEEL),
    ARMOR_PAIR(CONCRETE)
};

const TwoWayMap<MPHType, std::string> TdTypeConverter::Mph_Types {
    MPH_PAIR(IMMOBILE),
    MPH_PAIR(VERY_SLOW),
    MPH_PAIR(KINDA_SLOW),
    MPH_PAIR(SLOW),
    MPH_PAIR(SLOW_ISH),
    MPH_PAIR(MEDIUM_SLOW),
    MPH_PAIR(MEDIUM),
    MPH_PAIR(MEDIUM_FAST),
    MPH_PAIR(MEDIUM_FASTER),
    MPH_PAIR(FAST),
    MPH_PAIR(ROCKET),
    MPH_PAIR(VERY_FAST),
    MPH_PAIR(LIGHT_SPEED)
};

const TwoWayMap<WeaponType, std::string> TdTypeConverter::Weapon_Types {
    WEAPON_PAIR(NONE),
    WEAPON_PAIR(RIFLE),
    WEAPON_PAIR(CHAIN_GUN),
    WEAPON_PAIR(PISTOL),
    WEAPON_PAIR(M16),
    WEAPON_PAIR(DRAGON),
    WEAPON_PAIR(FLAMETHROWER),
    WEAPON_PAIR(FLAME_TONGUE),
    WEAPON_PAIR(CHEMSPRAY),
    { WEAPON_GRENADE, "GRENADE_WEAPON" },
    WEAPON_PAIR(75MM),
    WEAPON_PAIR(105MM),
    WEAPON_PAIR(120MM),
    WEAPON_PAIR(TURRET_GUN),
    WEAPON_PAIR(MAMMOTH_TUSK),
    { WEAPON_MLRS, "WEAPON_MLRS" },
    WEAPON_PAIR(155MM),
    WEAPON_PAIR(M60MG),
    WEAPON_PAIR(TOMAHAWK),
    WEAPON_PAIR(TOW_TWO),
    { WEAPON_NAPALM, "NAPALM_WEAPON" },
    WEAPON_PAIR(OBELISK_LASER),
    WEAPON_PAIR(NIKE),
    WEAPON_PAIR(HONEST_JOHN),
    { WEAPON_STEG, "STEG_WEAPON" },
    { WEAPON_TREX, "TREX_WEAPON" }
};

const TwoWayMap<HousesType, std::string> TdTypeConverter::House_Types {
    HOUSE_PAIR(NONE),
    HOUSE_PAIR(GOOD),
    HOUSE_PAIR(BAD),
    HOUSE_PAIR(NEUTRAL),
    HOUSE_PAIR(JP),
    HOUSE_PAIR(MULTI1),
    HOUSE_PAIR(MULTI2),
    HOUSE_PAIR(MULTI3),
    HOUSE_PAIR(MULTI4),
    HOUSE_PAIR(MULTI5),
    HOUSE_PAIR(MULTI6)
};

const TwoWayMap<StructType, std::string> TdTypeConverter::Struct_Types {
    STRUCT_PAIR(NONE),
    STRUCT_PAIR(WEAP),
    { STRUCT_GTOWER, "GTWR" },
    { STRUCT_ATOWER, "ATWR" },
    { STRUCT_OBELISK, "OBLI" },
    { STRUCT_RADAR, "HQ" },
    { STRUCT_TURRET, "GUN" },
    { STRUCT_CONST, "FACT" },
    { STRUCT_REFINERY, "PROC" },
    { STRUCT_STORAGE, "SILO" },
    { STRUCT_HELIPAD, "HPAD" },
    STRUCT_PAIR(SAM),
    { STRUCT_AIRSTRIP, "AFLD" },
    { STRUCT_POWER, "NUKE", },
    { STRUCT_ADVANCED_POWER, "NUK2" },
    { STRUCT_HOSPITAL, "HOSP" },
    { STRUCT_BARRACKS, "PYLE" },
    { STRUCT_TANKER, "ARCO" },
    { STRUCT_REPAIR, "FIX" },
    { STRUCT_BIO_LAB, "BIO" },
    STRUCT_PAIR(HAND),
    { STRUCT_TEMPLE, "TMPL" },
    STRUCT_PAIR(EYE),
    { STRUCT_MISSION, "MISS" },
    STRUCT_PAIR(V01),
    STRUCT_PAIR(V02),
    STRUCT_PAIR(V03),
    STRUCT_PAIR(V04),
    STRUCT_PAIR(V05),
    STRUCT_PAIR(V06),
    STRUCT_PAIR(V07),
    STRUCT_PAIR(V08),
    STRUCT_PAIR(V09),
    STRUCT_PAIR(V10),
    STRUCT_PAIR(V11),
    STRUCT_PAIR(V12),
    STRUCT_PAIR(V13),
    STRUCT_PAIR(V14),
    STRUCT_PAIR(V15),
    STRUCT_PAIR(V16),
    STRUCT_PAIR(V17),
    STRUCT_PAIR(V18),
    { STRUCT_PUMP, "V19" },
    STRUCT_PAIR(V20),
    STRUCT_PAIR(V21),
    STRUCT_PAIR(V22),
    STRUCT_PAIR(V23),
    STRUCT_PAIR(V24),
    STRUCT_PAIR(V25),
    STRUCT_PAIR(V26),
    STRUCT_PAIR(V27),
    STRUCT_PAIR(V28),
    STRUCT_PAIR(V29),
    STRUCT_PAIR(V30),
    STRUCT_PAIR(V31),
    STRUCT_PAIR(V32),
    STRUCT_PAIR(V33),
    STRUCT_PAIR(V34),
    STRUCT_PAIR(V35),
    STRUCT_PAIR(V36),
    STRUCT_PAIR(V37),
    { STRUCT_SANDBAG_WALL, "SBAG" },
    { STRUCT_CYCLONE_WALL, "CYCL" },
    { STRUCT_BRICK_WALL, "BRIK" },
    { STRUCT_BARBWIRE_WALL, "BARB" },
    { STRUCT_WOOD_WALL, "WOOD" }
};

const TwoWayMap<FactoryType, std::string> TdTypeConverter::Factory_Types {
    FACTORY_PAIR(NONE),
    FACTORY_PAIR(INFANTRY),
    FACTORY_PAIR(UNIT),
    FACTORY_PAIR(AIRCRAFT),
    FACTORY_PAIR(BUILDING)
};

const TwoWayMap<DirType, std::string> TdTypeConverter::Dir_Types {
    DIR_PAIR(N),
    DIR_PAIR(NE),
    DIR_PAIR(E),
    DIR_PAIR(SE),
    DIR_PAIR(S),
    DIR_PAIR(SW),
    DIR_PAIR(SW_X1),
    DIR_PAIR(SW_X2),
    DIR_PAIR(W),
    DIR_PAIR(NW)
};

const TwoWayMap<BSizeType, std::string> TdTypeConverter::BSize_Types {
    BSIZE_PAIR(NONE),
    BSIZE_PAIR(11),
    BSIZE_PAIR(21),
    BSIZE_PAIR(12),
    BSIZE_PAIR(22),
    BSIZE_PAIR(23),
    BSIZE_PAIR(32),
    BSIZE_PAIR(33),
    BSIZE_PAIR(42),
    BSIZE_PAIR(55)
};

const TwoWayMap<AircraftType, std::string> TdTypeConverter::Aircraft_Types {
    AIRCRAFT_PAIR(NONE),
    { AIRCRAFT_TRANSPORT, "TRAN" },
    AIRCRAFT_PAIR(A10),
    { AIRCRAFT_HELICOPTER, "HELI" },
    { AIRCRAFT_CARGO, "C17" },
    AIRCRAFT_PAIR(ORCA)
};

const TwoWayMap<MissionType, std::string> TdTypeConverter::Mission_Types {
    MISSION_PAIR(NONE),
    MISSION_PAIR(SLEEP),
    MISSION_PAIR(ATTACK),
    MISSION_PAIR(MOVE),
    MISSION_PAIR(RETREAT),
    MISSION_PAIR(GUARD),
    MISSION_PAIR(STICKY),
    MISSION_PAIR(ENTER),
    MISSION_PAIR(CAPTURE),
    MISSION_PAIR(HARVEST),
    MISSION_PAIR(GUARD_AREA),
    MISSION_PAIR(RETURN),
    MISSION_PAIR(STOP),
    MISSION_PAIR(AMBUSH),
    MISSION_PAIR(HUNT),
    MISSION_PAIR(TIMED_HUNT),
    MISSION_PAIR(UNLOAD),
    MISSION_PAIR(SABOTAGE),
    MISSION_PAIR(CONSTRUCTION),
    MISSION_PAIR(DECONSTRUCTION),
    MISSION_PAIR(REPAIR),
    MISSION_PAIR(RESCUE),
    MISSION_PAIR(MISSILE)
};

const TwoWayMap<AnimType, std::string> TdTypeConverter::Anim_Types {
    ANIM_PAIR(NONE),
    ANIM_PAIR(FBALL1),
    { ANIM_GRENADE, "GRENADEA" },
    ANIM_PAIR(FRAG1),
    ANIM_PAIR(FRAG2),
    ANIM_PAIR(VEH_HIT1),
    ANIM_PAIR(VEH_HIT2),
    ANIM_PAIR(VEH_HIT3),
    ANIM_PAIR(ART_EXP1),
    ANIM_PAIR(NAPALM1),
    ANIM_PAIR(NAPALM2),
    ANIM_PAIR(NAPALM3),
    { ANIM_SMOKE_PUFF, "SMOKEY" },
    ANIM_PAIR(PIFF),
    ANIM_PAIR(PIFFPIFF),
    ANIM_PAIR(FLAME_N),
    ANIM_PAIR(FLAME_NE),
    ANIM_PAIR(FLAME_E),
    ANIM_PAIR(FLAME_SE),
    ANIM_PAIR(FLAME_S),
    ANIM_PAIR(FLAME_SW),
    ANIM_PAIR(FLAME_W),
    ANIM_PAIR(FLAME_NW),
    ANIM_PAIR(CHEM_N),
    ANIM_PAIR(CHEM_NE),
    ANIM_PAIR(CHEM_E),
    ANIM_PAIR(CHEM_SE),
    ANIM_PAIR(CHEM_S),
    ANIM_PAIR(CHEM_SW),
    ANIM_PAIR(CHEM_W),
    ANIM_PAIR(CHEM_NW),
    { ANIM_FIRE_SMALL, "FIRE_S" },
    { ANIM_FIRE_MED, "FIRE_M" },
    { ANIM_FIRE_MED2, "FIRE_ME" },
    { ANIM_FIRE_TINY, "FIRE_T" },
    { ANIM_MUZZLE_FLASH, "GUNFIRE" },
    ANIM_PAIR(SMOKE_M),
    { ANIM_BURN_SMALL, "BURN_S" },
    { ANIM_BURN_MED, "BURN_M" },
    { ANIM_BURN_BIG, "BURN_L" },
    { ANIM_ON_FIRE_SMALL, "ONFIRE_S" },
    { ANIM_ON_FIRE_MED, "ONFIRE_M" },
    { ANIM_ON_FIRE_BIG, "ONFIRE_L" },
    ANIM_PAIR(SAM_N),
    ANIM_PAIR(SAM_NE),
    ANIM_PAIR(SAM_E),
    ANIM_PAIR(SAM_SE),
    ANIM_PAIR(SAM_S),
    ANIM_PAIR(SAM_SW),
    ANIM_PAIR(SAM_W),
    ANIM_PAIR(SAM_NW),
    ANIM_PAIR(GUN_N),
    ANIM_PAIR(GUN_NE),
    ANIM_PAIR(GUN_E),
    ANIM_PAIR(GUN_SE),
    ANIM_PAIR(GUN_S),
    ANIM_PAIR(GUN_SW),
    ANIM_PAIR(GUN_W),
    ANIM_PAIR(GUN_NW),
    ANIM_PAIR(LZ_SMOKE),
    { ANIM_ION_CANNON, "IONSFX" },
    { ANIM_ATOM_BLAST, "ATOMSFX" },
    { ANIM_CRATE_DEVIATOR, "DEVIATOR" },
    { ANIM_CRATE_DOLLAR, "DOLLAR" },
    { ANIM_CRATE_EARTH, "EARTH" },
    { ANIM_CRATE_EMPULSE, "EMPULSE" },
    { ANIM_CRATE_INVUN, "INVUN" },
    { ANIM_CRATE_MINE, "MINE" },
    { ANIM_CRATE_RAPID, "RAPID" },
    { ANIM_CRATE_STEALTH, "STEALTH" },
    { ANIM_CRATE_MISSILE, "MISSILE" },
    { ANIM_ATOM_DOOR, "ATOMDOOR" },
    { ANIM_MOVE_FLASH, "MV_FLASH" },
    { ANIM_OILFIELD_BURN, "FLMSPT" },
    ANIM_PAIR(TRIC_DIE),
    ANIM_PAIR(TREX_DIE),
    ANIM_PAIR(STEG_DIE),
    ANIM_PAIR(RAPT_DIE),
    { ANIM_CHEM_BALL, "CHEMBALL" },
    ANIM_PAIR(FLAG),
    ANIM_PAIR(BEACON),
    { ANIM_FIRE_SMALL_VIRTUAL, "FIRE_S_V" },
    { ANIM_FIRE_MED_VIRTUAL, "FIRE_M_V" },
    { ANIM_FIRE_MED2_VIRTUAL, "FIR2_M_V" },
    { ANIM_FIRE_TINY_VIRTUAL, "FIRE_T_V" },
    { ANIM_BEACON_VIRTUAL, "BEACON_V" }
};

const TwoWayMap<InfantryType, std::string> TdTypeConverter::Infantry_Types {
    INFANTRY_PAIR(NONE),
    INFANTRY_PAIR(E1),
    INFANTRY_PAIR(E2),
    INFANTRY_PAIR(E3),
    INFANTRY_PAIR(E4),
    INFANTRY_PAIR(E5),
    { INFANTRY_E7, "E6" },
    INFANTRY_PAIR(RAMBO),
    INFANTRY_PAIR(C1),
    INFANTRY_PAIR(C2),
    INFANTRY_PAIR(C3),
    INFANTRY_PAIR(C4),
    INFANTRY_PAIR(C5),
    INFANTRY_PAIR(C6),
    INFANTRY_PAIR(C7),
    INFANTRY_PAIR(C8),
    INFANTRY_PAIR(C9),
    INFANTRY_PAIR(C10),
    INFANTRY_PAIR(MOEBIUS),
    INFANTRY_PAIR(DELPHI),
    INFANTRY_PAIR(CHAN)
};

const TwoWayMap<UnitType, std::string> TdTypeConverter::Unit_Types {
    UNIT_PAIR(NONE),
    { UNIT_HTANK, "HTNK" },
    { UNIT_MTANK, "MTNK" },
    { UNIT_LTANK, "LTNK" },
    { UNIT_STANK, "STNK" },
    { UNIT_FTANK, "FTNK" },
    UNIT_PAIR(VICE),
    UNIT_PAIR(APC),
    { UNIT_MLRS, "MSAM" },
    UNIT_PAIR(JEEP),
    { UNIT_BUGGY, "BGGY" },
    { UNIT_HARVESTER, "HARV" },
    UNIT_PAIR(ARTY),
    { UNIT_MSAM, "MLRS" },
    { UNIT_HOVER, "LST" },
    UNIT_PAIR(MHQ),
    { UNIT_GUNBOAT, "BOAT" },
    UNIT_PAIR(MCV),
    UNIT_PAIR(BIKE),
    UNIT_PAIR(TRIC),
    UNIT_PAIR(TREX),
    UNIT_PAIR(RAPT),
    UNIT_PAIR(STEG)
};

const TwoWayMap<SpeedType, std::string> TdTypeConverter::Speed_Types {
    SPEED_PAIR(NONE),
    SPEED_PAIR(FOOT),
    SPEED_PAIR(TRACK),
    SPEED_PAIR(HARVESTER),
    SPEED_PAIR(WHEEL),
    SPEED_PAIR(WINGED),
    SPEED_PAIR(HOVER),
    SPEED_PAIR(FLOAT)
};

const TwoWayMap<BulletType, std::string> TdTypeConverter::Bullet_Types {
    BULLET_PAIR(NONE),
    BULLET_PAIR(SNIPER),
    BULLET_PAIR(BULLET),
    BULLET_PAIR(APDS),
    BULLET_PAIR(HE),
    BULLET_PAIR(SSM),
    BULLET_PAIR(SSM2),
    { BULLET_SAM, "SAM_BULT" },
    BULLET_PAIR(TOW),
    BULLET_PAIR(FLAME),
    { BULLET_CHEMSPRAY, "CHEMSPRA" },
    BULLET_PAIR(NAPALM),
    BULLET_PAIR(GRENADE),
    BULLET_PAIR(LASER),
    { BULLET_NUKE_UP, "ATOMICUP" },
    { BULLET_NUKE_DOWN, "ATOMICDN" },
    { BULLET_HONEST_JOHN, "HONESTJB" },
    { BULLET_SPREADFIRE, "SPRDFIRE" },
    { BULLET_HEADBUTT, "GORE" },
    { BULLET_TREXBITE, "CHEW" }
};

const TwoWayMap<WarheadType, std::string> TdTypeConverter::Warhead_Types {
    WARHEAD_PAIR(NONE),
    WARHEAD_PAIR(SA),
    { WARHEAD_HE, "HE_WARHEAD" },
    WARHEAD_PAIR(AP),
    WARHEAD_PAIR(FIRE),
    { WARHEAD_LASER, "LASER_WARHEAD" },
    WARHEAD_PAIR(PB),
    WARHEAD_PAIR(FIST),
    WARHEAD_PAIR(FOOT),
    WARHEAD_PAIR(HOLLOW_POINT),
    WARHEAD_PAIR(SPORE),
    WARHEAD_PAIR(HEADBUTT),
    WARHEAD_PAIR(FEEDME)
};
// VOC_BUILD_SELECT is omitted as it appears unused an is a pointer to another VOC (non-unique)
const TwoWayMap<VocType, std::string> TdTypeConverter::Voc_Types {
    VOC_PAIR(NONE),
    VOC_PAIR(RAMBO_PRESENT),
    VOC_PAIR(RAMBO_CMON),
    VOC_PAIR(RAMBO_UGOTIT),
    VOC_PAIR(RAMBO_COMIN),
    VOC_PAIR(RAMBO_LAUGH),
    VOC_PAIR(RAMBO_LEFTY),
    VOC_PAIR(RAMBO_NOPROB),
    VOC_PAIR(RAMBO_ONIT),
    VOC_PAIR(RAMBO_YELL),
    VOC_PAIR(RAMBO_ROCK),
    VOC_PAIR(RAMBO_TUFF),
    VOC_PAIR(RAMBO_YEA),
    VOC_PAIR(RAMBO_YES),
    VOC_PAIR(RAMBO_YO),
    VOC_PAIR(GIRL_OKAY),
    VOC_PAIR(GIRL_YEAH),
    VOC_PAIR(GUY_OKAY),
    VOC_PAIR(GUY_YEAH),
    VOC_PAIR(2DANGER),
    VOC_PAIR(ACKNOWL),
    VOC_PAIR(AFFIRM),
    VOC_PAIR(AWAIT),
    VOC_PAIR(MOVEOUT),
    VOC_PAIR(NEGATIVE),
    VOC_PAIR(NO_PROB),
    VOC_PAIR(READY),
    VOC_PAIR(REPORT),
    VOC_PAIR(RIGHT_AWAY),
    VOC_PAIR(ROGER),
    VOC_PAIR(UGOTIT),
    VOC_PAIR(UNIT),
    VOC_PAIR(VEHIC),
    VOC_PAIR(YESSIR),
    VOC_PAIR(BAZOOKA),
    VOC_PAIR(BLEEP),
    VOC_PAIR(BOMB1),
    VOC_PAIR(BUTTON),
    VOC_PAIR(RADAR_ON),
    VOC_PAIR(CONSTRUCTION),
    VOC_PAIR(CRUMBLE),
    VOC_PAIR(FLAMER1),
    VOC_PAIR(RIFLE),
    VOC_PAIR(M60),
    VOC_PAIR(GUN20),
    VOC_PAIR(M60A),
    VOC_PAIR(MINI),
    VOC_PAIR(RELOAD),
    VOC_PAIR(SLAM),
    VOC_PAIR(HVYGUN10),
    VOC_PAIR(ION_CANNON),
    VOC_PAIR(MGUN11),
    VOC_PAIR(MGUN2),
    VOC_PAIR(NUKE_FIRE),
    VOC_PAIR(NUKE_EXPLODE),
    VOC_PAIR(LASER),
    VOC_PAIR(LASER_POWER),
    VOC_PAIR(RADAR_OFF),
    VOC_PAIR(SNIPER),
    VOC_PAIR(ROCKET1),
    VOC_PAIR(ROCKET2),
    VOC_PAIR(MOTOR),
    VOC_PAIR(SCOLD),
    VOC_PAIR(SIDEBAR_OPEN),
    VOC_PAIR(SIDEBAR_CLOSE),
    VOC_PAIR(SQUISH2),
    VOC_PAIR(TANK1),
    VOC_PAIR(TANK2),
    VOC_PAIR(TANK3),
    VOC_PAIR(TANK4),
    VOC_PAIR(UP),
    VOC_PAIR(DOWN),
    VOC_PAIR(TARGET),
    VOC_PAIR(SONAR),
    VOC_PAIR(TOSS),
    VOC_PAIR(CLOAK),
    VOC_PAIR(BURN),
    VOC_PAIR(TURRET),
    VOC_PAIR(XPLOBIG4),
    VOC_PAIR(XPLOBIG6),
    VOC_PAIR(XPLOBIG7),
    VOC_PAIR(XPLODE),
    VOC_PAIR(XPLOS),
    VOC_PAIR(XPLOSML2),
    VOC_PAIR(SCREAM1),
    VOC_PAIR(SCREAM3),
    VOC_PAIR(SCREAM4),
    VOC_PAIR(SCREAM5),
    VOC_PAIR(SCREAM6),
    VOC_PAIR(SCREAM7),
    VOC_PAIR(SCREAM10),
    VOC_PAIR(SCREAM11),
    VOC_PAIR(SCREAM12),
    VOC_PAIR(YELL1),
    VOC_PAIR(YES),
    VOC_PAIR(COMMANDER),
    VOC_PAIR(HELLO),
    VOC_PAIR(HMMM),
    VOC_PAIR(EXCELLENT3),
    VOC_PAIR(OF_COURSE),
    VOC_PAIR(YESYES),
    VOC_PAIR(QUIP1),
    VOC_PAIR(THANKS),
    VOC_PAIR(CASHTURN),
    VOC_PAIR(BLEEPY3),
    VOC_PAIR(DINOMOUT),
    VOC_PAIR(DINOYES),
    VOC_PAIR(DINOATK1),
    VOC_PAIR(DINODIE1),
    VOC_PAIR(BEACON)
};

const TwoWayMap<PlayerColorType, std::string> TdTypeConverter::Player_Color_Types {
    PLAYER_COLOR_PAIR(NONE),
    PLAYER_COLOR_PAIR(GOLD),
    PLAYER_COLOR_PAIR(LTBLUE),
    PLAYER_COLOR_PAIR(RED),
    PLAYER_COLOR_PAIR(GREEN),
    PLAYER_COLOR_PAIR(ORANGE),
    PLAYER_COLOR_PAIR(BLUE)
};

const TwoWayMap<HouseColorType, std::string> TdTypeConverter::House_Color_Types {
    HOUSE_COLOR_PAIR(GOOD),
    HOUSE_COLOR_PAIR(BRIGHT_GOOD),
    HOUSE_COLOR_PAIR(BAD),
    HOUSE_COLOR_PAIR(BRIGHT_BAD),
    HOUSE_COLOR_PAIR(NEUTRAL),
    HOUSE_COLOR_PAIR(BRIGHT_NEUTRAL)
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

    throw std::invalid_argument("Unsupported SupportedByTdTypeConverter type - this is normally caused by concept being updated without updating supporting code");
}
