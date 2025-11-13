#include "common/stringutils.h"

#include "function.h"
#include "savegame.h"
#include "typeconverter.h"

using json = nlohmann::json;

#pragma region SaveGameHeader
void SaveGameHeader::ReadGlobals()
{
    ScenarioID = Scen.Scenario;
    PlayerHouseType = TdTypeConverter::To_String(PlayerPtr->Class->House);
}

bool SaveGameHeader::Validate() const
{
    auto result = true;

    if (CncStringUtils::Is_Blank(Version)) {
        CNC_LOGGER_ERROR("Blank/missing Version save game value");
        result = false;
    }

    if (!TdTypeConverter::Try_Parse<HousesType>(PlayerHouseType).has_value()) {
        CNC_LOGGER_ERROR("Invalid PlayerHouse save game value: {}", PlayerHouseType);
        result = false;
    }

    if (CncStringUtils::Is_Blank(Description)) {
        CNC_LOGGER_ERROR("Blank/missing Description save game value");
        result = false;
    }

    return result;
}

bool SaveGameHeader::WriteGlobals() const
{
    Scen.Scenario = ScenarioID;

    return true;
}
#pragma endregion

#pragma region SaveGameScenarioState
void SaveGameScenarioState::ReadGlobals()
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

    PlayerHouse = *PlayerPtr;

    SelectedObjects = {};

    for (auto i = 0; i < SelectedObjectsType::COUNT; i++) {
        DynamicVectorClass<ObjectClass*>& selection = CurrentObject.Raw(i);
        const auto count = selection.Count();
        std::vector<std::string> selectedObjectsEntry;

        for (auto j = 0; j < count; j++) {
            // TODO: Convert selected object to reference string
            // selectedObjectsEntry.emplace_back(selection[j]);
        }

        SelectedObjects.emplace_back(selectedObjectsEntry);
    }

    Waypoints = {};
    Waypoints.assign(Scen.Waypoint, Scen.Waypoint + std::size(Scen.Waypoint));
    Views = {};
    Views.assign(Scen.Views, Scen.Views + std::size(Scen.Views));
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

    for (const auto [field, value] : stringFields) {
        const auto bufferSize = GlobalBufferSizes.at(field);

        if (CncStringUtils::Is_Blank(value)) {
            CNC_LOGGER_ERROR("Blank/missing ScenarioState.{} save game value", field);

            result = false;
            continue;
        }

        if (bufferSize < value.length() + 1) {
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

    for (auto i = 0; i < SelectedObjectsType::COUNT; i++) {
        for (const auto& entry : SelectedObjects.at(i)) {
            // TODO: validate reference in entry
            // if (bad) { result = false; }
        }
    }

    return result;
}

bool SaveGameScenarioState::WriteGlobals() const
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

    // TODO: Delete and new before set?
    from_json(PlayerHouse, *PlayerPtr);

    for (auto i = 0; i < SelectedObjectsType::COUNT; i++) {
        DynamicVectorClass<ObjectClass*>& selection = CurrentObject.Raw(i);
        for (const auto& entry : SelectedObjects.at(i)) {
            // TODO: de-reference entry string
            // selection.Add(entry);

            // TODO: error handling on invalid reference
        }
    }

    std::ranges::copy(Waypoints, Scen.Waypoint);
    std::ranges::copy(Views, Scen.Views);

    return true;
}
#pragma endregion

#pragma region SaveGame
void SaveGame::ReadGlobals()
{
    Header.ReadGlobals();
    ScenarioState.ReadGlobals();
}

bool SaveGame::Validate() const
{
    // TODO: Validate Json Fields
    return Header.Validate() && ScenarioState.Validate();
}

bool SaveGame::WriteGlobals() const
{
    if (!Validate()) {
        return false;
    }

    Header.WriteGlobals();
    ScenarioState.WriteGlobals();

    return true;
}

std::string SaveGame::DumpJson() const
{
    const json save_json = *this;

    return save_json.dump();
}
#pragma endregion
