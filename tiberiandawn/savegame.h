#pragma once

#include <map>
#include <string>
#include <vector>

#include "common/logger.h"

#include "defines.h"
#include "house.h"

#define NAMEOF(SYMBOL) #SYMBOL

class SaveGameHeader
{
public:
    std::string Version;
    int ScenarioID;
    std::string PlayerHouse;
    std::string Description;

    void ReadGlobals();
    bool Validate() const;
    bool WriteGlobals() const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SaveGameHeader, Version, ScenarioID, PlayerHouse, Description)
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

    HouseClass* PlayerHouse;
    // TODO: Implement (How to resolve back? need to reference objects not copy)
    std::vector<std::vector<std::string>> SelectedObjects;
    std::vector<CELL> Waypoints;
    std::vector<CELL> Views;

    void ReadGlobals();
    bool Validate() const;
    bool WriteGlobals() const;

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
        // TODO: Implement
        // PlayerHouse,
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

class SaveGame
{
public:
    SaveGameHeader Header;
    std::string MapJson;
    std::string HousesJson;
    std::map<std::string, std::vector<std::string>> ObjectsJson;
    std::string LogicJson;
    std::vector<std::string> LayersJson;
    std::string ScoreJson;
    std::string AiBaseJson;
    SaveGameScenarioState ScenarioState;

    void ReadGlobals();
    bool Validate() const;
    bool WriteGlobals() const;
    std::string DumpJson() const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        SaveGame,
        Header,
        MapJson,
        HousesJson,
        ObjectsJson,
        LogicJson,
        LayersJson,
        ScoreJson,
        AiBaseJson,
        ScenarioState
    )
private:
    static inline const auto& Logger = CncLogger::For(SaveGame);

};
