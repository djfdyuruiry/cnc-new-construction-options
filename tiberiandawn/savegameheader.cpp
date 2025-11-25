#include "common/stringutils.h"

#include "function.h"
#include "savegameheader.h"
#include "typeconverter.h"

bool SaveGameHeader::From_Stream(std::ifstream& stream, SaveGameHeader& output)
{
    // read SaveGameHeader JSON
    std::string header_line;
    std::getline(stream, header_line, LINE_SEPERATOR);

    if (CncStringUtils::Is_Blank(header_line)) {
        CNC_LOGGER_ERROR("Save game is corrupt - {} JSON was not found", NAMEOF(SaveGameHeader));
        return false;
    }

    // parse JSON
    try {
        from_json(nlohmann::json::parse(header_line), output);

        return output.Validate();
    } catch (nlohmann::json::exception& e) {
        CNC_LOGGER_ERROR("Save game is corrupt, JSON parse error: {}", e.what());

        return false;
    }
}

bool SaveGameHeader::From_File(const std::string& path, SaveGameHeader& output)
{
    // build path using CDFileClass logic
    std::string full_path;

    CNC_LOGGER_INFO("Attempting to read header from JSON save game file: {}", path);

    if (CDFileClass file; !file.Open(path.c_str(), READ)) {
        CNC_LOGGER_ERROR("Failed to read full path to JSON save game");
        file.Close();
        return false;
    } else {
        full_path = std::string(file.File_Name());
        file.Close();
    }

    // open file stream
    auto save_file_stream = std::ifstream(full_path);

    return From_Stream(save_file_stream, output);
}

void SaveGameHeader::Read_Globals()
{
    ScenarioID = Scen.Scenario;
    PlayerHouseType = TdTypeConverter::To_String(PlayerPtr->Class->House);
    PlayerType = TdTypeConverter::To_String(ScenPlayer);
}

bool SaveGameHeader::Validate() const
{
    auto result = true;

    if (CncStringUtils::Is_Blank(Version)) {
        CNC_LOGGER_ERROR("Blank/missing Header.Version save game value");
        result = false;
    }

    if (!TdTypeConverter::Try_Parse<HousesType>(PlayerHouseType).has_value()) {
        CNC_LOGGER_ERROR("Invalid Header.PlayerHouse save game value: {}", PlayerHouseType);
        result = false;
    }

    if (!TdTypeConverter::Try_Parse<ScenarioPlayerType>(PlayerType).has_value()) {
        CNC_LOGGER_ERROR("Invalid Header.PlayerType save game value: {}", PlayerType);
        result = false;
    }

    if (CncStringUtils::Is_Blank(Description)) {
        CNC_LOGGER_ERROR("Blank/missing Header.Description save game value");
        result = false;
    }

    return result;
}

bool SaveGameHeader::Write_Globals() const
{
    Scen.Scenario = ScenarioID;
    ScenPlayer = Parse_Player_Type();
    Whom = Parse_Player_House_Type();

    return true;
}

HousesType SaveGameHeader::Parse_Player_House_Type() const
{
    return TdTypeConverter::Assert_Parse<HousesType>(
        PlayerHouseType,
        "Attempted to parse invalid Header.PlayerHouse save game value: {}"
    );
}

ScenarioPlayerType SaveGameHeader::Parse_Player_Type() const
{
    return TdTypeConverter::Assert_Parse<ScenarioPlayerType>(
        PlayerType,
        "Attempted to parse invalid Header.PlayerType save game value: {}"
    );
}

void SaveGameHeader::Dump_Json(std::string& output) const
{
    const nlohmann::json save_json = *this;

    output = save_json.dump();
}
