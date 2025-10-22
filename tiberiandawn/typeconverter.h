#pragma once

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "common/twowaymap.h"

#include "defines.h"

using ConverterTypeVariant = std::variant<
    ArmorType,
    MPHType,
    WeaponType,
    HousesType,
    StructType,
    FactoryType,
    DirType,
    BSizeType,
    AircraftType,
    MissionType,
    AnimType,
    InfantryType,
    UnitType,
    SpeedType,
    BulletType,
    WarheadType,
    VocType,
    PlayerColorType,
    HouseColorType
>;

template<typename T>
concept SupportedByTdTypeConverter = (
    std::is_same_v<T, ArmorType> ||
    std::is_same_v<T, MPHType> ||
    std::is_same_v<T, WeaponType> ||
    std::is_same_v<T, HousesType> ||
    std::is_same_v<T, StructType> ||
    std::is_same_v<T, FactoryType> ||
    std::is_same_v<T, DirType> ||
    std::is_same_v<T, BSizeType> ||
    std::is_same_v<T, AircraftType> ||
    std::is_same_v<T, MissionType> ||
    std::is_same_v<T, AnimType> ||
    std::is_same_v<T, InfantryType> ||
    std::is_same_v<T, UnitType> ||
    std::is_same_v<T, SpeedType> ||
    std::is_same_v<T, BulletType> ||
    std::is_same_v<T, WarheadType> ||
    std::is_same_v<T, VocType> ||
    std::is_same_v<T, PlayerColorType> ||
    std::is_same_v<T, HouseColorType>
);

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

class TdTypeConverter final
{
public:
    static inline const TwoWayMap<ArmorType, std::string> Armor_Types {
        ARMOR_PAIR(NONE),
        ARMOR_PAIR(WOOD),
        ARMOR_PAIR(ALUMINUM),
        ARMOR_PAIR(STEEL),
        ARMOR_PAIR(CONCRETE)
    };
    static inline const TwoWayMap<MPHType, std::string> Mph_Types {
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
    static inline const TwoWayMap<WeaponType, std::string> Weapon_Types {
        WEAPON_PAIR(NONE),
        WEAPON_PAIR(RIFLE),
        WEAPON_PAIR(CHAIN_GUN),
        WEAPON_PAIR(PISTOL),
        WEAPON_PAIR(M16),
        WEAPON_PAIR(DRAGON),
        WEAPON_PAIR(FLAMETHROWER),
        WEAPON_PAIR(FLAME_TONGUE),
        WEAPON_PAIR(CHEMSPRAY),
        WEAPON_PAIR(GRENADE),
        WEAPON_PAIR(75MM),
        WEAPON_PAIR(105MM),
        WEAPON_PAIR(120MM),
        WEAPON_PAIR(TURRET_GUN),
        WEAPON_PAIR(MAMMOTH_TUSK),
        WEAPON_PAIR(MLRS),
        WEAPON_PAIR(155MM),
        WEAPON_PAIR(M60MG),
        WEAPON_PAIR(TOMAHAWK),
        WEAPON_PAIR(TOW_TWO),
        WEAPON_PAIR(NAPALM),
        WEAPON_PAIR(OBELISK_LASER),
        WEAPON_PAIR(NIKE),
        WEAPON_PAIR(HONEST_JOHN),
        WEAPON_PAIR(STEG),
        WEAPON_PAIR(TREX)
    };
    static inline const TwoWayMap<HousesType, std::string> House_Types {
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
    static inline const TwoWayMap<StructType, std::string> Struct_Types {
        STRUCT_PAIR(NONE),
        STRUCT_PAIR(WEAP),
        STRUCT_PAIR(GTOWER),
        STRUCT_PAIR(ATOWER),
        STRUCT_PAIR(OBELISK),
        STRUCT_PAIR(RADAR),
        STRUCT_PAIR(TURRET),
        STRUCT_PAIR(CONST),
        STRUCT_PAIR(REFINERY),
        STRUCT_PAIR(STORAGE),
        STRUCT_PAIR(HELIPAD),
        STRUCT_PAIR(SAM),
        STRUCT_PAIR(AIRSTRIP),
        STRUCT_PAIR(POWER),
        STRUCT_PAIR(ADVANCED_POWER),
        STRUCT_PAIR(HOSPITAL),
        STRUCT_PAIR(BARRACKS),
        STRUCT_PAIR(TANKER),
        STRUCT_PAIR(REPAIR),
        STRUCT_PAIR(BIO_LAB),
        STRUCT_PAIR(HAND),
        STRUCT_PAIR(TEMPLE),
        STRUCT_PAIR(EYE),
        STRUCT_PAIR(MISSION),
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
        STRUCT_PAIR(PUMP),
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
        STRUCT_PAIR(SANDBAG_WALL),
        STRUCT_PAIR(CYCLONE_WALL),
        STRUCT_PAIR(BRICK_WALL),
        STRUCT_PAIR(BARBWIRE_WALL),
        STRUCT_PAIR(WOOD_WALL)
    };
    static inline const TwoWayMap<FactoryType, std::string> Factory_Types {
        FACTORY_PAIR(NONE),
        FACTORY_PAIR(INFANTRY),
        FACTORY_PAIR(UNIT),
        FACTORY_PAIR(AIRCRAFT),
        FACTORY_PAIR(BUILDING)
    };
    static inline const TwoWayMap<DirType, std::string> Dir_Types {
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
    static inline const TwoWayMap<BSizeType, std::string> BSize_Types {
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
    static inline const TwoWayMap<AircraftType, std::string> Aircraft_Types {
        AIRCRAFT_PAIR(NONE),
        AIRCRAFT_PAIR(TRANSPORT),
        AIRCRAFT_PAIR(A10),
        AIRCRAFT_PAIR(HELICOPTER),
        AIRCRAFT_PAIR(CARGO),
        AIRCRAFT_PAIR(ORCA)
    };
    static inline const TwoWayMap<MissionType, std::string> Mission_Types {
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
    static inline const TwoWayMap<AnimType, std::string> Anim_Types {
        ANIM_PAIR(NONE),
        ANIM_PAIR(FBALL1),
        ANIM_PAIR(GRENADE),
        ANIM_PAIR(FRAG1),
        ANIM_PAIR(FRAG2),
        ANIM_PAIR(VEH_HIT1),
        ANIM_PAIR(VEH_HIT2),
        ANIM_PAIR(VEH_HIT3),
        ANIM_PAIR(ART_EXP1),
        ANIM_PAIR(NAPALM1),
        ANIM_PAIR(NAPALM2),
        ANIM_PAIR(NAPALM3),
        ANIM_PAIR(SMOKE_PUFF),
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
        ANIM_PAIR(FIRE_SMALL),
        ANIM_PAIR(FIRE_MED),
        ANIM_PAIR(FIRE_MED2),
        ANIM_PAIR(FIRE_TINY),
        ANIM_PAIR(MUZZLE_FLASH),
        ANIM_PAIR(SMOKE_M),
        ANIM_PAIR(BURN_SMALL),
        ANIM_PAIR(BURN_MED),
        ANIM_PAIR(BURN_BIG),
        ANIM_PAIR(ON_FIRE_SMALL),
        ANIM_PAIR(ON_FIRE_MED),
        ANIM_PAIR(ON_FIRE_BIG),
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
        ANIM_PAIR(ION_CANNON),
        ANIM_PAIR(ATOM_BLAST),
        ANIM_PAIR(CRATE_DEVIATOR),
        ANIM_PAIR(CRATE_DOLLAR),
        ANIM_PAIR(CRATE_EARTH),
        ANIM_PAIR(CRATE_EMPULSE),
        ANIM_PAIR(CRATE_INVUN),
        ANIM_PAIR(CRATE_MINE),
        ANIM_PAIR(CRATE_RAPID),
        ANIM_PAIR(CRATE_STEALTH),
        ANIM_PAIR(CRATE_MISSILE),
        ANIM_PAIR(ATOM_DOOR),
        ANIM_PAIR(MOVE_FLASH),
        ANIM_PAIR(OILFIELD_BURN),
        ANIM_PAIR(TRIC_DIE),
        ANIM_PAIR(TREX_DIE),
        ANIM_PAIR(STEG_DIE),
        ANIM_PAIR(RAPT_DIE),
        ANIM_PAIR(CHEM_BALL),
        ANIM_PAIR(FLAG),
        ANIM_PAIR(BEACON),
        ANIM_PAIR(FIRE_SMALL_VIRTUAL),
        ANIM_PAIR(FIRE_MED_VIRTUAL),
        ANIM_PAIR(FIRE_MED2_VIRTUAL),
        ANIM_PAIR(FIRE_TINY_VIRTUAL),
        ANIM_PAIR(BEACON_VIRTUAL)
    };
    static inline const TwoWayMap<InfantryType, std::string> Infantry_Types {
        INFANTRY_PAIR(NONE),
        INFANTRY_PAIR(E1),
        INFANTRY_PAIR(E2),
        INFANTRY_PAIR(E3),
        INFANTRY_PAIR(E4),
        INFANTRY_PAIR(E5),
        INFANTRY_PAIR(E7),
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
    static inline const TwoWayMap<UnitType, std::string> Unit_Types {
        UNIT_PAIR(NONE),
        UNIT_PAIR(HTANK),
        UNIT_PAIR(MTANK),
        UNIT_PAIR(LTANK),
        UNIT_PAIR(STANK),
        UNIT_PAIR(FTANK),
        UNIT_PAIR(VICE),
        UNIT_PAIR(APC),
        UNIT_PAIR(MLRS),
        UNIT_PAIR(JEEP),
        UNIT_PAIR(BUGGY),
        UNIT_PAIR(HARVESTER),
        UNIT_PAIR(ARTY),
        UNIT_PAIR(MSAM),
        UNIT_PAIR(HOVER),
        UNIT_PAIR(MHQ),
        UNIT_PAIR(GUNBOAT),
        UNIT_PAIR(MCV),
        UNIT_PAIR(BIKE),
        UNIT_PAIR(TRIC),
        UNIT_PAIR(TREX),
        UNIT_PAIR(RAPT),
        UNIT_PAIR(STEG)
    };
    static inline const TwoWayMap<SpeedType, std::string> Speed_Types {
        SPEED_PAIR(NONE),
        SPEED_PAIR(FOOT),
        SPEED_PAIR(TRACK),
        SPEED_PAIR(HARVESTER),
        SPEED_PAIR(WHEEL),
        SPEED_PAIR(WINGED),
        SPEED_PAIR(HOVER),
        SPEED_PAIR(FLOAT)
    };
    static inline const TwoWayMap<BulletType, std::string> Bullet_Types {
        BULLET_PAIR(NONE),
        BULLET_PAIR(SNIPER),
        BULLET_PAIR(BULLET),
        BULLET_PAIR(APDS),
        BULLET_PAIR(HE),
        BULLET_PAIR(SSM),
        BULLET_PAIR(SSM2),
        BULLET_PAIR(SAM),
        BULLET_PAIR(TOW),
        BULLET_PAIR(FLAME),
        BULLET_PAIR(CHEMSPRAY),
        BULLET_PAIR(NAPALM),
        BULLET_PAIR(GRENADE),
        BULLET_PAIR(LASER),
        BULLET_PAIR(NUKE_UP),
        BULLET_PAIR(NUKE_DOWN),
        BULLET_PAIR(HONEST_JOHN),
        BULLET_PAIR(SPREADFIRE),
        BULLET_PAIR(HEADBUTT),
        BULLET_PAIR(TREXBITE)
    };
    static inline const TwoWayMap<WarheadType, std::string> Warhead_Types {
        WARHEAD_PAIR(NONE),
        WARHEAD_PAIR(SA),
        WARHEAD_PAIR(HE),
        WARHEAD_PAIR(AP),
        WARHEAD_PAIR(FIRE),
        WARHEAD_PAIR(LASER),
        WARHEAD_PAIR(PB),
        WARHEAD_PAIR(FIST),
        WARHEAD_PAIR(FOOT),
        WARHEAD_PAIR(HOLLOW_POINT),
        WARHEAD_PAIR(SPORE),
        WARHEAD_PAIR(HEADBUTT),
        WARHEAD_PAIR(FEEDME)
    };
    // VOC_BUILD_SELECT is omitted as it appears unused an is a pointer to another VOC (non-unique)
    static inline const TwoWayMap<VocType, std::string> Voc_Types {
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
    static inline const TwoWayMap<PlayerColorType, std::string> Player_Color_Types {
        PLAYER_COLOR_PAIR(NONE),
        PLAYER_COLOR_PAIR(GOLD),
        PLAYER_COLOR_PAIR(LTBLUE),
        PLAYER_COLOR_PAIR(RED),
        PLAYER_COLOR_PAIR(GREEN),
        PLAYER_COLOR_PAIR(ORANGE),
        PLAYER_COLOR_PAIR(BLUE)
    };
    static inline const TwoWayMap<HouseColorType, std::string> House_Color_Types {
        HOUSE_COLOR_PAIR(GOOD),
        HOUSE_COLOR_PAIR(BRIGHT_GOOD),
        HOUSE_COLOR_PAIR(BAD),
        HOUSE_COLOR_PAIR(BRIGHT_BAD),
        HOUSE_COLOR_PAIR(NEUTRAL),
        HOUSE_COLOR_PAIR(BRIGHT_NEUTRAL)
    };

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::string To_String(T instance)
    {
        if constexpr (std::is_same_v<T, ArmorType>) {
            return Armor_Types[instance].value_or(
                Armor_Types[ARMOR_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, MPHType>) {
            return Mph_Types[instance].value_or(
                Mph_Types[MPH_IMMOBILE].value()
            );
        } else if constexpr (std::is_same_v<T, WeaponType>) {
            return Weapon_Types[instance].value_or(
                Weapon_Types[WEAPON_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, HousesType>) {
            return House_Types[instance].value_or(
                House_Types[HOUSE_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, StructType>) {
            return Struct_Types[instance].value_or(
                Struct_Types[STRUCT_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, FactoryType>) {
            return Factory_Types[instance].value_or(
                Factory_Types[FACTORY_TYPE_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, DirType>) {
            return Dir_Types[instance].value_or(
                Dir_Types[DIR_N].value()
            );
        } else if constexpr (std::is_same_v<T, BSizeType>) {
            return BSize_Types[instance].value_or(
                BSize_Types[BSIZE_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, AircraftType>) {
            return Aircraft_Types[instance].value_or(
                Aircraft_Types[AIRCRAFT_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, MissionType>) {
            return Mission_Types[instance].value_or(
                Mission_Types[MISSION_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, AnimType>) {
            return Anim_Types[instance].value_or(
                Anim_Types[ANIM_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, InfantryType>) {
            return Infantry_Types[instance].value_or(
                Infantry_Types[INFANTRY_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, UnitType>) {
            return Unit_Types[instance].value_or(
                Unit_Types[UNIT_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, SpeedType>) {
            return Speed_Types[instance].value_or(
                Speed_Types[SPEED_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, BulletType>) {
            return Bullet_Types[instance].value_or(
                Bullet_Types[BULLET_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, WarheadType>) {
            return Warhead_Types[instance].value_or(
                Warhead_Types[WARHEAD_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, VocType>) {
            return Voc_Types[instance].value_or(
                Voc_Types[VOC_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, PlayerColorType>) {
            return Player_Color_Types[instance].value_or(
                Player_Color_Types[REMAP_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, HouseColorType>) {
            return House_Color_Types[instance].value_or(
                House_Color_Types[HOUSE_COLOR_GOOD].value()
            );
        }

        throw std::invalid_argument("Unsupported SupportedByTdTypeConverter type - this is normally caused by concept being updated without updating supporting code");
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::string To_Csv_String(const std::vector<T>& instances)
    {
        std::ostringstream oss;
        auto first = true;

        for (const auto& instance : instances) {
            if (!first) {
                oss << ",";
            }

            oss << To_String<T>(instance);
            first = false;
        }

        return oss.str();
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::optional<T> Try_Parse(std::string str)
    {
        if (str.empty() || std::all_of(str.begin(), str.end(), [](auto c) { return std::isspace(c); }))
        {
            return std::nullopt;
        }

        if constexpr (std::is_same_v<T, ArmorType>) {
            return Armor_Types[str];
        } else if constexpr (std::is_same_v<T, MPHType>) {
            return Mph_Types[str];
        } else if constexpr (std::is_same_v<T, WeaponType>) {
            return Weapon_Types[str];
        } else if constexpr (std::is_same_v<T, HousesType>) {
            return House_Types[str];
        } else if constexpr (std::is_same_v<T, StructType>) {
            return Struct_Types[str];
        } else if constexpr (std::is_same_v<T, FactoryType>) {
            return Factory_Types[str];
        } else if constexpr (std::is_same_v<T, DirType>) {
            return Dir_Types[str];
        } else if constexpr (std::is_same_v<T, BSizeType>) {
            return BSize_Types[str];
        } else if constexpr (std::is_same_v<T, AircraftType>) {
            return Aircraft_Types[str];
        } else if constexpr (std::is_same_v<T, MissionType>) {
            return Mission_Types[str];
        } else if constexpr (std::is_same_v<T, AnimType>) {
            return Anim_Types[str];
        } else if constexpr (std::is_same_v<T, InfantryType>) {
            return Infantry_Types[str];
        } else if constexpr (std::is_same_v<T, UnitType>) {
            return Unit_Types[str];
        } else if constexpr (std::is_same_v<T, SpeedType>) {
            return Speed_Types[str];
        } else if constexpr (std::is_same_v<T, BulletType>) {
            return Bullet_Types[str];
        } else if constexpr (std::is_same_v<T, WarheadType>) {
            return Warhead_Types[str];
        } else if constexpr (std::is_same_v<T, VocType>) {
            return Voc_Types[str];
        } else if constexpr (std::is_same_v<T, PlayerColorType>) {
            return Player_Color_Types[str];
        } else if constexpr (std::is_same_v<T, HouseColorType>) {
            return House_Color_Types[str];
        }

        throw std::invalid_argument("Unsupported SupportedByTdTypeConverter type - this is normally caused by concept being updated without updating supporting code");
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::vector<T> Try_Parse_Csv(const std::string& csv_str, const char delimiter = ',')
    {
        std::vector<T> instances;
        size_t start = 0;
        size_t end = csv_str.find(delimiter);

        while (end != std::string::npos) {
            if (auto instance = Try_Parse<T>(csv_str.substr(start, end - start)); instance.has_value()) {
                instances.push_back(instance.value());
            }

            start = end + 1;
            end = csv_str.find(delimiter, start);
        }

        if (auto instance = Try_Parse<T>(csv_str.substr(start)); instance.has_value()) {
            instances.push_back(instance.value());
        }

        return instances;
    }

private:
    TdTypeConverter() = delete;

};

// IniRuleContext macro 'method' for loading types that are converted from string representation to a non-trivial type
#define Load_With_TdConverter(TYPE, VAR) \
    Load_With_Converter_Callback<TYPE, TdTypeConverter>(#VAR, VAR, [&](auto v) { VAR = v; })

// IniRuleContext macro 'method' for loading types that are converted from string representation to a list of non-trivial type instances
#define Load_Csv_With_TdConverter(TYPE, VAR) \
    Load_With_Csv_Converter_Callback<TYPE, TdTypeConverter>(#VAR, VAR, [&](auto v) { VAR = std::move(v); })
