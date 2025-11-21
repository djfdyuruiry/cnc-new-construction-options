#include "common/stringutils.h"

#include "function.h"
#include "savegame.h"
#include "typeconverter.h"

#include <fstream>

#pragma region SaveGameHeader
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

#pragma endregion

#pragma region SaveGameScenarioState
void SaveGameScenarioState::Read_Globals()
{
    ScenarioNumber = Scen.Scenario;
    ScenarioFileName = Scen.FileName;
    ScenarioDirection = TdTypeConverter::To_String(ScenDir);
    ScenarioVariation = TdTypeConverter::To_String(ScenVar);
    BriefText = Scen.BriefingText;
    BriefMovieName = BriefMovie;
    WinMovieName = WinMovie;
    LoseMovieName = LoseMovie;
    ActionMovieName = ActionMovie;
    CarryOverMoney = Scen.CarryOverMoney;
    CarryOverPercent = Scen.CarryOverPercent;
    BuildLevelNumber = BuildLevel;

    Difficulty = TdTypeConverter::To_String(Scen.Difficulty);
    AiDifficulty = TdTypeConverter::To_String(Scen.CDifficulty);

    FrameNumber = Frame;
    EndCountdownNumber = EndCountDown;
    HasTempleBeenHitWithIonCannon = TempleIoned;
    AreThingiesEnabledFlag = AreThingiesEnabled;

    SelectedObjects = CurrentObject;
    Waypoints = Scen.Waypoint;
    Views = Scen.Views;
}

bool SaveGameScenarioState::Validate() const
{
    auto result = true;

    if (!TdTypeConverter::Try_Parse<ScenarioDirType>(ScenarioDirection)) {
        CNC_LOGGER_ERROR("Invalid ScenarioState.ScenarioDirection save game value: {}", ScenarioDirection);
        result = false;
    }

    if (!TdTypeConverter::Try_Parse<ScenarioVarType>(ScenarioVariation)) {
        CNC_LOGGER_ERROR("Invalid ScenarioState.ScenarioVariation save game value: {}", ScenarioVariation);
        result = false;
    }

    const std::map<std::string, std::string> stringFields = {
        { NAMEOF(ScenarioFileName), ScenarioFileName },
        { NAMEOF(BriefText), BriefText },
        { NAMEOF(BriefMovieName), BriefMovieName },
        { NAMEOF(WinMovieName), WinMovieName },
        { NAMEOF(LoseMovieName), LoseMovieName },
        { NAMEOF(ActionMovieName), ActionMovieName }
    };

    for (const auto& [field, value] : stringFields) {
        const auto bufferSize = GlobalBufferSizes.at(field);

        if (CncStringUtils::Is_Blank(value)) {
            CNC_LOGGER_ERROR("Blank/missing ScenarioState.{} save game value", field);

            result = false;
        } else if (bufferSize < value.length() + 1) {
            CNC_LOGGER_ERROR(
                "Invalid ScenarioState.{} save game value '{}', value is longer than max allowed size: {}",
                field,
                value.length(),
                bufferSize
            );

            result = false;
        }
    }

    if (!TdTypeConverter::Try_Parse<DiffType>(Difficulty).has_value()) {
        CNC_LOGGER_ERROR("Unable to parse ScenarioState.Difficulty save game value: {}", Difficulty);
        result = false;
    }

    if (!TdTypeConverter::Try_Parse<DiffType>(AiDifficulty).has_value()) {
        CNC_LOGGER_ERROR("Unable to parse ScenarioState.AiDifficulty save game value: {}", AiDifficulty);
        result = false;
    }

    if (!TdTypeConverter::Try_Parse<DiffType>(AiDifficulty).has_value()) {
        CNC_LOGGER_ERROR("Unable to parse ScenarioState.AiDifficulty save game value: {}", AiDifficulty);
        result = false;
    }

    if (!SelectedObjects.is_object()) {
        CNC_LOGGER_ERROR(
            "Invalid ScenarioState.SelectedObjects save game value, expected object - actual type: {}",
            SelectedObjects.type_name()
        );
        result = false;
    }

    if (!Waypoints.is_array()) {
        CNC_LOGGER_ERROR(
            "Invalid ScenarioState.Waypoints save game value, expected array - actual type: {}",
            SelectedObjects.type_name()
        );
        result = false;
    } else if (Waypoints.size() > std::size(Scen.Waypoint)) {
        CNC_LOGGER_ERROR(
            "Invalid ScenarioState.Waypoints save game value, expected an array with max {} elements - actual size: {}",
            std::size(Scen.Waypoint),
            SelectedObjects.size()
        );
        result = false;
    }

    if (!Views.is_array()) {
        CNC_LOGGER_ERROR(
            "Invalid ScenarioState.Views save game value, expected array - actual type: {}",
            SelectedObjects.type_name()
        );
        result = false;
    } else if (Views.size() > std::size(Scen.Views)) {
        CNC_LOGGER_ERROR(
            "Invalid ScenarioState.Views save game value, expected an array with max {} elements - actual size: {}",
            std::size(Scen.Views),
            SelectedObjects.size()
        );
        result = false;
    }

    return result;
}

ScenarioDirType SaveGameScenarioState::Parse_Scenario_Direction() const
{
    return TdTypeConverter::Assert_Parse<ScenarioDirType>(
        ScenarioDirection,
        "Attempted to parse invalid ScenarioState.ScenarioDirection save game value: {}"
    );
}

ScenarioVarType SaveGameScenarioState::Parse_Scenario_Variation() const
{
    return TdTypeConverter::Assert_Parse<ScenarioVarType>(
        ScenarioVariation,
        "Attempted to parse invalid ScenarioState.ScenarioVariation save game value: {}"
    );
}

bool SaveGameScenarioState::Write_Globals() const
{
    if (!Validate()) {
        return false;
    }

    Scen.Scenario = ScenarioNumber;

    // validate already called, so we are using known valid values
    ScenDir = TdTypeConverter::Try_Parse<ScenarioDirType>(ScenarioDirection).value();
    ScenVar = TdTypeConverter::Try_Parse<ScenarioVarType>(ScenarioVariation).value();
    strcpy(Scen.FileName, ScenarioFileName.c_str());
    strcpy(Scen.BriefingText, BriefText.c_str());
    strcpy(BriefMovie, BriefMovieName.c_str());
    strcpy(WinMovie, WinMovieName.c_str());
    strcpy(LoseMovie, LoseMovieName.c_str());
    strcpy(ActionMovie, ActionMovieName.c_str());

    Scen.CarryOverMoney = CarryOverMoney;
    Scen.CarryOverPercent = CarryOverPercent;
    BuildLevel = BuildLevelNumber;

    // validate already called, so we are unpacking known valid values
    Scen.Difficulty = TdTypeConverter::Try_Parse<DiffType>(Difficulty).value();
    Scen.CDifficulty = TdTypeConverter::Try_Parse<DiffType>(AiDifficulty).value();

    Frame = FrameNumber;
    EndCountDown = EndCountdownNumber;
    TempleIoned = HasTempleBeenHitWithIonCannon;
    AreThingiesEnabled = AreThingiesEnabledFlag;

    from_json(SelectedObjects, CurrentObject);
    from_json(Waypoints, Scen.Waypoint);
    from_json(Views, Scen.Views);

    if (!CncStringUtils::Is_Blank(ScenarioFileName)) {
        // TODO: Could change this to save the rules cache/RulesClass instance and rehydrate it (portable rules + preservation)
        if (CCFileClass ini_file(ScenarioFileName.c_str()); ini_file.Is_Available()) {
            if (CCINIClass ini; ini.Load(ini_file, true) != 0) {
                Rule.Init(ini);
                Rule.Init_Types(ini);
            } else {
                CNC_LOG_ERROR(
                    "Failed to load scenario INI filename stored in save game JSON: {}",
                    Scen.FileName
                );
            }
        } else {
            CNC_LOG_WARN("Scenario INI file recorded in save game JSON was not found: {}");
        }
    } else {
        CNC_LOG_DEBUG("No scenario INI filename found in save game JSON");
    }

    return true;
}
#pragma endregion

#pragma region SaveGameObjectHeaps
void SaveGameObjectHeaps::Read_Globals()
{
    // see globals.cpp for heap declarations
    AnimsHeap = Anims;
    AircraftHeap = Aircraft;
    BulletsHeap = Bullets;
    BuildingsHeap = Buildings;
    FactoriesHeap = Factories;
    HousesHeap = Houses;
    InfantryHeap = Infantry;
    OverlaysHeap = Overlays;
    SmudgesHeap = Smudges;
    TemplatesHeap = Templates;
    TerrainsHeap = Terrains;
    TeamTypesHeap = TeamTypes;
    TeamsHeap = Teams;
    TriggersHeap = Triggers;
    UnitsHeap = Units;
}

bool SaveGameObjectHeaps::Validate() const
{
    auto result = true;

    std::map<std::string_view, const nlohmann::json*> heaps = {
        { NAMEOF(AnimsHeap), &AnimsHeap },
        { NAMEOF(AircraftHeap), &AircraftHeap },
        { NAMEOF(BulletsHeap), &BulletsHeap },
        { NAMEOF(BuildingsHeap), &BuildingsHeap },
        { NAMEOF(FactoriesHeap), &FactoriesHeap },
        { NAMEOF(HousesHeap), &HousesHeap },
        { NAMEOF(InfantryHeap), &InfantryHeap },
        { NAMEOF(OverlaysHeap), &OverlaysHeap },
        { NAMEOF(SmudgesHeap), &SmudgesHeap },
        { NAMEOF(TemplatesHeap), &TemplatesHeap },
        { NAMEOF(TerrainsHeap), &TerrainsHeap },
        { NAMEOF(TeamTypesHeap), &TeamTypesHeap },
        { NAMEOF(TeamsHeap), &TeamsHeap },
        { NAMEOF(TriggersHeap), &TriggersHeap },
        { NAMEOF(UnitsHeap), &UnitsHeap }
    };

    for (const auto& [name, heap_ptr] : heaps) {
        const auto& heap = *heap_ptr;

        if (!heap.is_object()) {
            result = false;
            CNC_LOGGER_ERROR(
                "Invalid Objects.{} save game value - json object expected, actual type: {}",
                name,
                heap.type_name()
            );
        } else if (std::ranges::distance(heap.items()) < 1) {
            CNC_LOGGER_WARN("Objects.{} save game value has no entries", name);
        }
    }

    return result;
}

bool SaveGameObjectHeaps::Write_Globals() const
{
    if (!Validate()) {
        return false;
    }

    from_json(AnimsHeap, Anims);
    from_json(AircraftHeap, Aircraft);
    from_json(BulletsHeap, Aircraft);
    from_json(BuildingsHeap, Buildings);
    from_json(FactoriesHeap, Factories);
    from_json(HousesHeap, Houses);
    from_json(InfantryHeap, Infantry);
    from_json(OverlaysHeap, Overlays);
    from_json(SmudgesHeap, Smudges);
    from_json(TemplatesHeap, Templates);
    from_json(TerrainsHeap, Terrains);
    from_json(TeamTypesHeap, TeamTypes);
    from_json(TeamsHeap, Teams);
    from_json(TriggersHeap, Triggers);
    from_json(UnitsHeap, Units);

    return true;
}
#pragma endregion

#pragma region SaveGame
bool SaveGame::From_File(const std::string& path, SaveGame& output)
{
    std::string full_path;

    if (CDFileClass file; !file.Open(path.c_str(), READ)) {
        CNC_LOGGER_ERROR("Failed to open JSON save game file");
        file.Close();
        return false;
    } else {
        full_path = std::string(file.File_Name());
        file.Close();
    }

    auto save_file_stream = std::ifstream(full_path);
    const auto save_json = nlohmann::json::parse(save_file_stream);

    output = save_json.get<SaveGame>();

    return true;
}


void SaveGame::Read_Globals()
{
    Header.Read_Globals();
    ScenarioState.Read_Globals();
    Objects.Read_Globals();

    GameMap = Map;
    GameLogic = Logic;
    Layers = DisplayClass::Layer;
    AiBase = Base;
    GameScore = Score;
}

bool SaveGame::Validate() const
{
    auto result = true;

    result = Header.Validate() && result;
    result = ScenarioState.Validate() && result;
    result = Objects.Validate() && result;

    if (!GameMap.is_object()) {
        result = false;
        CNC_LOGGER_ERROR(
            "Invalid {} save game value - json object expected, actual type: {}",
            NAMEOF(GameMap),
            GameMap.type_name()
        );
    }

    if (!GameLogic.is_object()) {
        result = false;
        CNC_LOGGER_ERROR(
            "Invalid {} save game value - json object expected, actual type: {}",
            NAMEOF(GameLogic),
            GameLogic.type_name()
        );
    }

    if (!Layers.is_array()) {
        result = false;
        CNC_LOGGER_ERROR(
            "Invalid {} save game value - json array expected, actual type: {}",
            NAMEOF(Layers),
            Layers.type_name()
        );
    } else if (Layers.size() > std::size(DisplayClass::Layer)) {
        result = false;
        CNC_LOGGER_ERROR(
            "Invalid {} save game value - json array with max size {} expected, actual size: {}",
            NAMEOF(Layers),
            std::size(DisplayClass::Layer),
            Layers.size()
        );
    }

    if (!AiBase.is_object()) {
        result = false;
        CNC_LOGGER_ERROR(
            "Invalid {} save game value - json object expected, actual type: {}",
            NAMEOF(AiBase),
            AiBase.type_name()
        );
    }

    if (!GameScore.is_object()) {
        result = false;
        CNC_LOGGER_ERROR(
            "Invalid {} save game value - json object expected, actual type: {}",
            NAMEOF(GameScore),
            GameScore.type_name()
        );
    }

    return result;
}

bool SaveGame::Write_Globals() const
{
    if (!Validate()) {
        return false;
    }

    Clear_Scenario();

    const auto result = Header.Write_Globals() &&
        ScenarioState.Write_Globals() &&
        Objects.Write_Globals();

    // Map
    Map.Free_Cells();
    Map.Alloc_Cells();

    from_json(GameMap, reinterpret_cast<MouseClass&>(Map));

    if (Map.Theater != LastTheater) {
        Reset_Theater_Shapes();
    }

    Map.Init_Theater(Map.Theater);
    TerrainTypeClass::Init(Map.Theater);
    TemplateTypeClass::Init(Map.Theater);
    OverlayTypeClass::Init(Map.Theater);
    UnitTypeClass::Init(Map.Theater);
    InfantryTypeClass::Init(Map.Theater);
    BuildingTypeClass::Init(Map.Theater);
    BulletTypeClass::Init(Map.Theater);
    AnimTypeClass::Init(Map.Theater);
    AircraftTypeClass::Init(Map.Theater);
    SmudgeTypeClass::Init(Map.Theater);

    LastTheater = Map.Theater;

    from_json(GameLogic, Logic);
    from_json(Layers, DisplayClass::Layer);
    from_json(AiBase, Base);
    from_json(GameScore, Score);

    Decode_All_Pointers();

    PlayerPtr = HouseClass::As_Pointer(
        Header.Parse_Player_House_Type()
    );

    return result;
}

void SaveGame::Dump_Json(std::string& output) const
{
    const nlohmann::json save_json = *this;

    output = save_json.dump();
}
#pragma endregion
