#pragma once

#include <fstream>
#include <string>

#include "common/json.h"
#include "common/logger.h"

#include "defines.h"

class SaveGameHeader final
{
public:
    // Keep a consistent line seperator, ensuring save files are platform-agnostic.
    // (JSON standard already ensures all text is UTF-8, so we just need to worry about this)
    static constexpr char LINE_SEPERATOR = '\n';

    std::string Version;
    int ScenarioID;
    std::string PlayerHouseType;
    std::string PlayerType;
    std::string Description;

    static bool From_Stream(std::ifstream& stream, SaveGameHeader& output);
    static bool From_File(const std::string& path, SaveGameHeader& output);

    void Read_Globals();
    bool Validate() const;
    bool Write_Globals() const;

    HousesType Parse_Player_House_Type() const;
    ScenarioPlayerType Parse_Player_Type() const;

    void Dump_Json(std::string& output) const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SaveGameHeader, Version, ScenarioID, PlayerHouseType, PlayerType, Description)

private:
    static inline const auto& Logger = CncLogger::For(SaveGameHeader);
};
