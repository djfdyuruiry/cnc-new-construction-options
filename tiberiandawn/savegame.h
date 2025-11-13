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

    void ReadGlobals();
    bool Validate() const;
    bool WriteGlobals() const;

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

    json PlayerHouse;
    // TODO: Implement (How to resolve back? need to reference objects not copy)
    std::vector<std::vector<TARGET>> SelectedObjects;
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
        PlayerHouse,
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
    SaveGameScenarioState ScenarioState;

    json Map;
    std::vector<json> Layers;

    json Houses;
    json AiBase;

    std::map<std::string, std::vector<json>> Objects;

    json Logic;
    json Score;

    void ReadGlobals();
    bool Validate() const;
    bool WriteGlobals() const;
    std::string DumpJson() const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        SaveGame,
        Header,
        Map,
        Houses,
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
