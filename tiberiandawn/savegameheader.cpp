#include "common/b64straw.h"
#include "common/stringutils.h"

#include "function.h"
#include "savegameheader.h"

#include "typeconverter.h"

bool SaveGameData::Apply_Rules(RulesClass& rules) const
{
    CNC_LOGGER_INFO("Applying scenario rules from save game data");

    std::string error_message;

    try {
        from_json(ScenarioRules, rules);
        return true;
    } catch (const CncJsonException& e) {
        error_message = e.what();
    } catch (const nlohmann::json::exception& e) {
        error_message = e.what();
    }

    CNC_LOG_ERROR("Error applying rules from save game data: {}", error_message);
    return false;
}

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
    std::string error_message;

    try {
        from_json(nlohmann::json::parse(header_line), output);

        const auto validate_result =  output.Validate();

        if (!validate_result) {
            CNC_LOGGER_ERROR("Save game header is invalid");
        }

        return validate_result;
    } catch (const CncJsonException& e) {
        error_message = e.what();
    } catch (const nlohmann::json::exception& e) {
        error_message = e.what();
    }

    CNC_LOGGER_ERROR("Save game is corrupt, JSON parse error: {}", error_message);
    return false;
}

bool SaveGameHeader::From_File(const std::string& path, SaveGameHeader& output)
{
    std::string full_path;

    CNC_LOGGER_INFO("Attempting to read header from JSON save game file: {}", path);

    // build path using CDFileClass logic
    CDFileClass file(path.c_str());

    if (!file.Is_Available()) {
        return false;
    }

    if (!file.Open(READ)) {
        CNC_LOGGER_ERROR("Failed to read full path to JSON save game");
        file.Close();
        return false;
    }

    full_path = std::string(file.File_Name());
    file.Close();

    // open file stream
    auto save_file_stream = std::ifstream(full_path);

    return From_Stream(save_file_stream, output);
}

void SaveGameHeader::Read_Globals()
{
    ScenarioGameType = TdTypeConverter::To_String(GameToPlay);
    ScenarioID = Scen.Scenario;
    PlayerHouseType = TdTypeConverter::To_String(PlayerPtr->Class->House);
    PlayerType = TdTypeConverter::To_String(ScenPlayer);
}

/**
 * Read screenshot taken before options dialog was shown into a base64 string.
 */
void SaveGameHeader::Read_Screenshot_If_Present()
{
    static const auto screenshot_path = PathsClass::Concatenate_Paths(
        Paths.User_Screenshot_Path(),
        PRE_DIALOG_SCREENSHOT_FILE_NAME
    );

    CCFileClass screenshot;

    const auto screenshot_available = screenshot.Open(screenshot_path.c_str());

    ScreenshotBase64.clear();

    if (!screenshot_available) {
        return;
    }

    FileStraw binary_in(screenshot);
    Base64Straw base64_out(Base64Straw::ENCODE);

    base64_out.Get_From(binary_in);

    while (true) {
        char buffer[512];

        const auto length = base64_out.Get(buffer, sizeof(buffer) - 1);

        buffer[length] = '\0';

        if (length == 0) {
            break;
        }

        ScreenshotBase64 += buffer;
    }
}

bool SaveGameHeader::Validate() const
{
    auto result = true;

    if (CncStringUtils::Is_Blank(Version)) {
        CNC_LOGGER_ERROR("Blank/missing Header.Version save game value");
        result = false;
    }

    if (!TdTypeConverter::Try_Parse<GameType>(ScenarioGameType)) {
        CNC_LOGGER_ERROR("Invalid ScenarioState.ScenarioGameType save game value: {}", ScenarioGameType);
        result = false;
    }

    if (!TdTypeConverter::Try_Parse<HousesType>(PlayerHouseType).has_value()) {
        CNC_LOGGER_ERROR("Invalid Header.PlayerHouse save game value: {}", PlayerHouseType);
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
    GameToPlay = Parse_Game_Type();
    ScenPlayer = Parse_Player_Type();
    Whom = Parse_Player_House_Type();

    return true;
}

std::optional<std::string> SaveGameHeader::Write_Screenshot_If_Present() const
{
    static const auto screenshot_path = PathsClass::Concatenate_Paths(
        Paths.User_Screenshot_Path(),
        ".save-load.png"
    );

    if (ScreenshotBase64.empty()) {
        return std::nullopt;
    }

    CCFileClass screenshot;

    screenshot.Set_Name(screenshot_path.c_str());

    if (screenshot.Is_Available() && !screenshot.Delete()) {
        CNC_LOGGER_ERROR("Failed to clear previous save game screenshot: {}", screenshot_path);
        return std::nullopt;
    }

    screenshot.Open(WRITE);

    BufferStraw string_in(ScreenshotBase64.c_str(), ScreenshotBase64.size());

    Base64Straw binary_out(Base64Straw::DECODE);
    binary_out.Get_From(string_in);

    while (true) {
        char buffer[512];

        const auto length = binary_out.Get(buffer, sizeof(buffer) - 1);

        if (length == 0) {
            break;
        }

        screenshot.Write(buffer, length);
    }

    screenshot.Close();

    return screenshot_path;
}

void SaveGameHeader::Set_SaveGameData(SaveGameData data)
{
    SaveData = std::move(data);
}

const SaveGameData& SaveGameHeader::Get_SaveGameData() const
{
    if (!SaveData.has_value()) {
        throw std::runtime_error("Attempt was made to access save game header with empty save data export");
    }

    return SaveData.value();
}

GameType SaveGameHeader::Parse_Game_Type() const
{
    return TdTypeConverter::Assert_Parse<GameType>(
        ScenarioGameType,
        "Attempted to parse invalid Header.ScenarioGameType save game value: {}"
    );
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
