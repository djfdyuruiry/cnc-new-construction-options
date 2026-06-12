#pragma once

#include <fstream>
#include <string>

#include "common/json.h"
#include "common/logger.h"

#include "defines.h"

class SaveGameData final
{
public:
    SpecialClass SkirmishSpecial;
    bool SkirmishSuperweaponsEnabled;
    std::vector<std::string> ScenarioScripts;
    nlohmann::json ScenarioRules;

    SaveGameData(SpecialClass skirmish_special,
                 bool skirmish_superweapons_enabled,
                 std::vector<std::string> scenario_scripts,
                 nlohmann::json scenario_rules)
        : SkirmishSpecial(std::move(skirmish_special))
        , SkirmishSuperweaponsEnabled(skirmish_superweapons_enabled)
        , ScenarioScripts(std::move(scenario_scripts))
        , ScenarioRules(std::move(scenario_rules))
    {
    }

     bool Apply_Rules(RulesClass& rules) const;

private:
    static inline const auto& Logger = CncLogger::For(SaveGameData);
};

class SaveGameHeader final
{
public:
    // Keep a consistent line seperator, ensuring save files are platform-agnostic.
    // (JSON standard already ensures all text is UTF-8, so we just need to worry about this)
    static constexpr char LINE_SEPERATOR = '\n';

    std::string Version;
    std::string ScenarioGameType;
    int ScenarioID;
    std::string PlayerHouseType;
    std::string PlayerType;
    std::string Description;

    static bool From_Stream(std::ifstream& stream, SaveGameHeader& output);
    static bool From_File(const std::string& path, SaveGameHeader& output);

    void Read_Globals();
    bool Validate() const;
    bool Write_Globals() const;
    void Set_SaveGameData(SaveGameData data);
    const SaveGameData& Get_SaveGameData() const;

    GameType Parse_Game_Type() const;
    HousesType Parse_Player_House_Type() const;
    ScenarioPlayerType Parse_Player_Type() const;

    void Dump_Json(std::string& output) const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        SaveGameHeader,
        Version,
        ScenarioGameType,
        ScenarioID,
        PlayerHouseType,
        PlayerType,
        Description
    )

private:
    static inline const auto& Logger = CncLogger::For(SaveGameHeader);
    std::optional<SaveGameData> SaveData;
};
