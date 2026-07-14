#pragma once

#include "defines.h"
#include "door.h"
#include "teamtype.h"
#include "trigger.h"

// convenience macro for using commas as a parameter to other macros
#define MACRO_COMMA ,

// convenience macro to allow re-using the tiberian dawn enum type list to create variants and concepts
#define TD_ENUMS_FORMAT(PREFIX, POSTFIX, POSTFIX_LAST) \
    PREFIX AircraftType POSTFIX \
    PREFIX AnimType POSTFIX \
    PREFIX ArmorType POSTFIX \
    PREFIX BulletType POSTFIX \
    PREFIX BSizeType POSTFIX \
    PREFIX BStateType POSTFIX \
    PREFIX CCPaletteType POSTFIX \
    PREFIX CloakType POSTFIX \
    PREFIX DiffType POSTFIX \
    PREFIX DirType POSTFIX \
    PREFIX DoorClass::DoorStateType POSTFIX \
    PREFIX DoType POSTFIX \
    PREFIX EventType POSTFIX \
    PREFIX FacingType POSTFIX \
    PREFIX FactoryType POSTFIX \
    PREFIX GameType POSTFIX \
    PREFIX HouseColorType POSTFIX \
    PREFIX HousesType POSTFIX \
    PREFIX InfantryType POSTFIX \
    PREFIX KeyNumType POSTFIX \
    PREFIX KindType POSTFIX \
    PREFIX LandType POSTFIX \
    PREFIX LayerType POSTFIX \
    PREFIX MissionType POSTFIX \
    PREFIX MouseType POSTFIX \
    PREFIX MPHType POSTFIX \
    PREFIX OverlayType POSTFIX \
    PREFIX PlayerColorType POSTFIX \
    PREFIX RadarEnum POSTFIX \
    PREFIX RadioMessageType POSTFIX \
    PREFIX RTTIType POSTFIX \
    PREFIX ScenarioDirType POSTFIX \
    PREFIX ScenarioPlayerType POSTFIX \
    PREFIX ScenarioVarType POSTFIX \
    PREFIX SpecialWeaponType POSTFIX \
    PREFIX SmudgeType POSTFIX \
    PREFIX SourceType POSTFIX \
    PREFIX SpeedType POSTFIX \
    PREFIX StateType POSTFIX \
    PREFIX StructType POSTFIX \
    PREFIX TeamMissionType POSTFIX \
    PREFIX TemplateType POSTFIX \
    PREFIX TerrainType POSTFIX \
    PREFIX TheaterType POSTFIX \
    PREFIX TriggerClass::ActionType POSTFIX \
    PREFIX TriggerClass::PersistantType POSTFIX \
    PREFIX UnitType POSTFIX \
    PREFIX UrgencyType POSTFIX \
    PREFIX VocType POSTFIX \
    PREFIX VoxType POSTFIX \
    PREFIX WarheadType POSTFIX \
    PREFIX WeaponType POSTFIX \
    PREFIX ZoneType POSTFIX_LAST

// type constraint for converter methods
template<typename T>
concept SupportedByTdTypeConverter = (
    TD_ENUMS_FORMAT(std::is_same_v<T MACRO_COMMA, > ||, >)
);

// variant alternative to SupportedByTdTypeConverter
using ConverterTypeVariant = std::variant<
    TD_ENUMS_FORMAT(, MACRO_COMMA, )
>;

// used to enable storing string conversion maps for each enum type in a stl container
using EnumTwoWayMapVariant = std::variant<
    TD_ENUMS_FORMAT(TwoWayMap<, MACRO_COMMA std::string>MACRO_COMMA, MACRO_COMMA std::string>)
>;
