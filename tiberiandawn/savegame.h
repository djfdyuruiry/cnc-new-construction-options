#pragma once

#include <map>
#include <string>
#include <vector>

#include "common/json.h"
#include "common/logger.h"

#include "defines.h"

class SaveGameHeader
{
public:
    std::string Version;
    int ScenarioID;
    std::string PlayerHouseType;
    std::string Description;

    void Read_Globals();
    bool Validate() const;
    bool Write_Globals() const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SaveGameHeader, Version, ScenarioID, PlayerHouseType, Description)

private:
    static inline const auto& Logger = CncLogger::For(SaveGameHeader);
};

class SaveGameScenarioState
{
public:
    int ScenarioNumber;
    std::string ScenarioDirection;
    std::string ScenarioVariation;
    std::string ScenarioFileName;
    std::string BriefText;
    std::string BriefMovieName;
    std::string WinMovieName;
    std::string LoseMovieName;
    std::string ActionMovieName;
    int CarryOverMoney;
    int CarryOverPercent;
    unsigned int BuildLevelNumber;

    std::string Difficulty;
    std::string AiDifficulty;

    int FrameNumber;
    int EndCountdownNumber;
    bool HasTempleBeenHitWithIonCannon;
    int AreThingiesEnabledFlag;

    std::vector<std::vector<TARGET>> SelectedObjects;
    std::vector<CELL> Waypoints;
    std::vector<CELL> Views;

    void Read_Globals();
    bool Validate() const;
    bool Write_Globals() const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        SaveGameScenarioState,
        ScenarioNumber,
        ScenarioDirection,
        ScenarioVariation,
        ScenarioFileName,
        BriefText,
        BriefMovieName,
        WinMovieName,
        LoseMovieName,
        ActionMovieName,
        CarryOverMoney,
        CarryOverPercent,
        BuildLevelNumber,
        Difficulty,
        AiDifficulty,
        FrameNumber,
        EndCountdownNumber,
        HasTempleBeenHitWithIonCannon,
        AreThingiesEnabledFlag,
        SelectedObjects,
        Waypoints,
        Views
    )

private:
    static inline const auto& Logger = CncLogger::For(SaveGameScenarioState);
    static inline const std::map<std::string, int> GlobalBufferSizes = {
        { NAMEOF(ScenarioFileName), std::size(Scen.FileName) },
        { NAMEOF(BriefText), std::size(Scen.BriefingText) },
        { NAMEOF(BriefMovieName), std::size(BriefMovie) },
        { NAMEOF(WinMovieName), std::size(WinMovie) },
        { NAMEOF(LoseMovieName), std::size(LoseMovie) },
        { NAMEOF(ActionMovieName), std::size(ActionMovie) }
    };
};

class SaveGameObjectHeaps
{
public:
    nlohmann::json AnimsHeap;
    nlohmann::json AircraftHeap;
    nlohmann::json BulletsHeap;
    nlohmann::json BuildingsHeap;
    nlohmann::json FactoriesHeap;
    nlohmann::json HousesHeap;
    nlohmann::json InfantryHeap;
    nlohmann::json OverlaysHeap;
    nlohmann::json SmudgesHeap;
    nlohmann::json TemplatesHeap;
    nlohmann::json TerrainsHeap;
    nlohmann::json TeamTypesHeap;
    nlohmann::json TeamsHeap;
    nlohmann::json TriggersHeap;
    nlohmann::json UnitsHeap;

    void Read_Globals();
    bool Validate() const;
    bool Write_Globals() const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        SaveGameObjectHeaps,
        AnimsHeap,
        AircraftHeap,
        BulletsHeap,
        BuildingsHeap,
        FactoriesHeap,
        HousesHeap,
        InfantryHeap,
        OverlaysHeap,
        SmudgesHeap,
        TemplatesHeap,
        TerrainsHeap,
        TeamTypesHeap,
        TeamsHeap,
        TriggersHeap,
        UnitsHeap
    )

private:
    static inline const auto& Logger = CncLogger::For(SaveGameObjectHeaps);
};

class SaveGame
{
public:
    SaveGameHeader Header;
    SaveGameScenarioState ScenarioState;

    nlohmann::json GameMap;
    std::vector<nlohmann::json> Layers;

    SaveGameObjectHeaps Objects;

    nlohmann::json AiBase;
    nlohmann::json Logic;
    nlohmann::json Score;

    void Read_Globals();
    bool Validate() const;
    bool Write_Globals() const;
    void Dump_Json(std::string& output) const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        SaveGame,
        Header,
        GameMap,
        Objects,
        Logic,
        Layers,
        Score,
        AiBase,
        ScenarioState
    )

private:
    static inline const auto& Logger = CncLogger::For(SaveGame);
};
