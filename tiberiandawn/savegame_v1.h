#pragma once

#include <fstream>
#include <string>

#include "common/cdfile.h"
#include "common/json.h"
#include "common/logger.h"

#include "defines.h"
#include "savegameheader.h"

/**
 * Various metadata about the current scenario, consisting of INI
 * file data, game settings and game state.
 */
class SaveGameScenarioState_v1
{
public:

    // ini file values

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

    // game settings

    std::string Difficulty;
    std::string AiDifficulty;

    // game state

    int FrameNumber;
    int EndCountdownNumber;
    bool HasTempleBeenHitWithIonCannon;
    int AreThingiesEnabledFlag;

    // skirmish state

    std::string MultiPlayerName;
    std::string MultiPlayerPrefColor;
    std::string MultiPlayerHouse;
    int MultiPlayerCount;
    int MultiPlayerGhosts;
    bool MultiPlayerBases;
    int MultiPlayerCredits;
    int MultiPlayerTiberium;
    int MultiPlayerGoodies;
    int MultiPlayerSolo;
    int MultiPlayerUnitCount;
    unsigned char MultiPlayerLocalID;
    nlohmann::json MultiPlayerIds;
    nlohmann::json MultiPlayerNames;
    nlohmann::json MultiPlayerHouses;
    nlohmann::json MultiSpecial;
    bool MultiSuperweaponsEnabled;

    // map state

    nlohmann::json SelectedObjects;
    nlohmann::json Waypoints;
    nlohmann::json Views;

    void Read_Globals();
    bool Validate(GameType scenario_game_type) const;
    ScenarioDirType Parse_Scenario_Direction() const;
    ScenarioVarType Parse_Scenario_Variation() const;
    SpecialClass Parse_MultiPlayer_Special() const;
    bool Write_Globals() const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        SaveGameScenarioState_v1,
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
        MultiPlayerName,
        MultiPlayerPrefColor,
        MultiPlayerHouse,
        MultiPlayerCount,
        MultiPlayerGhosts,
        MultiPlayerBases,
        MultiPlayerCredits,
        MultiPlayerTiberium,
        MultiPlayerGoodies,
        MultiPlayerSolo,
        MultiPlayerUnitCount,
        MultiPlayerLocalID,
        MultiPlayerIds,
        MultiPlayerNames,
        MultiPlayerHouses,
        MultiSpecial,
        MultiSuperweaponsEnabled,
        SelectedObjects,
        Waypoints,
        Views
    )

private:
    static inline const auto& Logger = CncLogger::For(SaveGameScenarioState_v1);
    static inline const std::map<std::string, int> GlobalBufferSizes = {
        { NAMEOF(ScenarioFileName), std::size(Scen.FileName) },
        { NAMEOF(BriefText), std::size(Scen.BriefingText) },
        { NAMEOF(BriefMovieName), std::size(BriefMovie) },
        { NAMEOF(WinMovieName), std::size(WinMovie) },
        { NAMEOF(LoseMovieName), std::size(LoseMovie) },
        { NAMEOF(ActionMovieName), std::size(ActionMovie) }
    };
};

/**
 * C&C uses TFixedIHeapClass instances to store objects in heaps, grouped by
 * the type of the object (Unit, Infantry etc.). This class records the values
 * of these heaps as JSON.
 */
class SaveGameObjectHeaps_v1
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
        SaveGameObjectHeaps_v1,
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
    static inline const auto& Logger = CncLogger::For(SaveGameObjectHeaps_v1);
};

#ifdef REMASTER_BUILD
/**
 * C&C remastered uses the DLLExportClass Save/Load methods to write extra data to binary save.
 * This class is to model and replicate this logic for the new JSON save format.
 */
class SaveGameRemasterState_v1
{
public:
    nlohmann::json RemasterMultiplayerStartPositions;
    nlohmann::json RemasterPlayerIDs;
    int RemasterClientSidebarWidthInLeptons;
    nlohmann::json RemasterMPlayerIsHuman;
    nlohmann::json RemasterPlacementType;
    nlohmann::json RemasterMultiplayerSidebars;

    void Read_Dll_State();
    bool Validate() const;
    bool Write_Dll_State() const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        SaveGameRemasterState_v1,
        RemasterMultiplayerStartPositions,
        RemasterPlayerIDs,
        RemasterClientSidebarWidthInLeptons,
        RemasterMPlayerIsHuman,
        RemasterPlacementType,
        RemasterMultiplayerSidebars
    )

private:
    static inline const auto& Logger = CncLogger::For(SaveGameRemasterState_v1);
};
#endif

/**
 * Version 1 of the JSON save game format. New versions should inherit this class. New versions should provide sane
 * defaults for new values. Any new behaviour should also be backwards compatible with previous versions.
 */
class SaveGame_v1
{
public:
    static constexpr std::string_view Version_Name = "v1";

    SaveGameScenarioState_v1 ScenarioState;
    SaveGameObjectHeaps_v1 Objects;

    // triggers

    nlohmann::json GameCellTriggers;
    nlohmann::json GameHouseTriggers;
    nlohmann::json RemovedTriggers;

    // map objects

    nlohmann::json GameMap;
    nlohmann::json GameLogic;
    nlohmann::json Layers;
    nlohmann::json AiBase;

    // score tracking
    nlohmann::json GameScore;

#ifdef REMASTER_BUILD
    SaveGameRemasterState_v1 RemasterState;
#endif

    bool Load_From_File(const std::string& path);
    void Read_Globals();
    bool Validate(GameType scenario_game_type) const;
    bool Write_Globals() const;
    void Dump_Json(std::string& output) const;
    bool To_File(CDFileClass& save_file, const SaveGameHeader& header) const;

    /**
     * Header is stored separately from main JSON object to allow reading header
     * info without parsing entire JSON structure.
     */
#ifndef REMASTER_BUILD
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        SaveGame_v1,
        ScenarioState,
        Objects,
        GameCellTriggers,
        GameHouseTriggers,
        RemovedTriggers,
        GameMap,
        GameLogic,
        Layers,
        AiBase,
        GameScore
    )
#else
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        SaveGame_v1,
        ScenarioState,
        Objects,
        GameCellTriggers,
        GameHouseTriggers,
        RemovedTriggers,
        GameMap,
        GameLogic,
        Layers,
        AiBase,
        GameScore,
        RemasterState
    )
#endif

private:
    static inline const auto& Logger = CncLogger::For(SaveGame_v1);
};
