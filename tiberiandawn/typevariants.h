#pragma once

#include "defines.h"
#include "door.h"
#include "teamtype.h"
#include "trigger.h"

template<typename T>
concept SupportedByTdTypeConverter = (
    std::is_same_v<T, AircraftType> ||
    std::is_same_v<T, AnimType> ||
    std::is_same_v<T, ArmorType> ||
    std::is_same_v<T, BulletType> ||
    std::is_same_v<T, BSizeType> ||
    std::is_same_v<T, BStateType> ||
    std::is_same_v<T, CCPaletteType> ||
    std::is_same_v<T, CloakType> ||
    std::is_same_v<T, DiffType> ||
    std::is_same_v<T, DirType> ||
    std::is_same_v<T, DoorClass::DoorStateType> ||
    std::is_same_v<T, DoType> ||
    std::is_same_v<T, EventType> ||
    std::is_same_v<T, FacingType> ||
    std::is_same_v<T, FactoryType> ||
    std::is_same_v<T, GameType> ||
    std::is_same_v<T, HouseColorType> ||
    std::is_same_v<T, HousesType> ||
    std::is_same_v<T, InfantryType> ||
    std::is_same_v<T, KeyNumType> ||
    std::is_same_v<T, KindType> ||
    std::is_same_v<T, LandType> ||
    std::is_same_v<T, LayerType> ||
    std::is_same_v<T, MissionType> ||
    std::is_same_v<T, MouseType> ||
    std::is_same_v<T, MPHType> ||
    std::is_same_v<T, OverlayType> ||
    std::is_same_v<T, PlayerColorType> ||
    std::is_same_v<T, RadarEnum> ||
    std::is_same_v<T, RadioMessageType> ||
    std::is_same_v<T, RTTIType> ||
    std::is_same_v<T, ScenarioDirType> ||
    std::is_same_v<T, ScenarioPlayerType> ||
    std::is_same_v<T, ScenarioVarType> ||
    std::is_same_v<T, SpecialWeaponType> ||
    std::is_same_v<T, SmudgeType> ||
    std::is_same_v<T, SourceType> ||
    std::is_same_v<T, SpeedType> ||
    std::is_same_v<T, StateType>||
    std::is_same_v<T, StructType> ||
    std::is_same_v<T, TeamMissionType> ||
    std::is_same_v<T, TemplateType> ||
    std::is_same_v<T, TerrainType> ||
    std::is_same_v<T, TheaterType> ||
    std::is_same_v<T, TriggerClass::ActionType> ||
    std::is_same_v<T, TriggerClass::PersistantType> ||
    std::is_same_v<T, UnitType> ||
    std::is_same_v<T, UrgencyType> ||
    std::is_same_v<T, VocType> ||
    std::is_same_v<T, VoxType> ||
    std::is_same_v<T, WarheadType> ||
    std::is_same_v<T, WeaponType> ||
    std::is_same_v<T, ZoneType>
);

// Matches the SupportedByTdTypeConverter Concept types
using ConverterTypeVariant = std::variant<
    AircraftType,
    AnimType,
    ArmorType,
    BulletType,
    BSizeType,
    BStateType,
    CCPaletteType,
    CloakType,
    DiffType,
    DirType,
    DoorClass::DoorStateType,
    DoType,
    EventType,
    FacingType,
    FactoryType,
    GameType,
    HouseColorType,
    HousesType,
    InfantryType,
    KeyNumType,
    KindType,
    LandType,
    LayerType,
    MissionType,
    MouseType,
    MPHType,
    OverlayType,
    PlayerColorType,
    RadarEnum,
    RadioMessageType,
    RTTIType,
    ScenarioDirType,
    ScenarioPlayerType,
    ScenarioVarType,
    SpecialWeaponType,
    SmudgeType,
    SourceType,
    SpeedType,
    StateType,
    StructType,
    TeamMissionType,
    TemplateType,
    TerrainType,
    TheaterType,
    TriggerClass::ActionType,
    TriggerClass::PersistantType,
    UnitType,
    UrgencyType,
    VocType,
    VoxType,
    WarheadType,
    WeaponType,
    ZoneType
>;
