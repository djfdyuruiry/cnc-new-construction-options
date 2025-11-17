#include "common/stringutils.h"

#include "function.h"
#include "savegame.h"
#include "typeconverter.h"

#pragma region SaveGameHeader
void SaveGameHeader::Read_Globals()
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

bool SaveGameHeader::Write_Globals() const
{
    Scen.Scenario = ScenarioID;

    return true;
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

    SelectedObjects = {};

    for (auto i = 0; i < SelectedObjectsType::COUNT; i++) {
        DynamicVectorClass<ObjectClass*>& selection = CurrentObject.Raw(i);
        const auto count = selection.Count();
        std::vector<TARGET> selectedObjectsEntry;

        for (auto j = 0; j < count; j++) {
            selectedObjectsEntry.emplace_back(selection[j]->As_Target());
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

    return result;
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

    for (auto i = 0; i < SelectedObjectsType::COUNT; i++) {
        DynamicVectorClass<ObjectClass*>& selection = CurrentObject.Raw(i);
        for (const auto& entry : SelectedObjects.at(i)) {
            selection.Add((ObjectClass*)(intptr_t)(entry));
        }
    }

    std::ranges::copy(Waypoints, Scen.Waypoint);
    std::ranges::copy(Views, Scen.Views);

    return true;
}
#pragma endregion

#pragma region SaveGameObjectHeaps
void SaveGameObjectHeaps::Read_Globals()
{
    // TODO: remaining heaps

    // see globals.cpp for heap declarations
    //AnimsHeap = Anims;
    //AircraftHeap = Aircraft;
    //BulletsHeap = Bullets;
    //BuildingsHeap = Buildings;
    //FactoriesHeap = Factories;
    HousesHeap = Houses;
    //InfantryHeap = Infantry;
    //OverlaysHeap = Overlays;
    //SmudgesHeap = Smudges;
    //TemplatesHeap = Templates;
    //TerrainsHeap = Terrains;
    TeamTypesHeap = TeamTypes;
    //TeamsHeap = Teams;
    //TriggersHeap = Triggers;
    //UnitsHeap = Units;
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

    //Anims = AnimsHeap;
    //Aircraft = AircraftHeap;
    //Bullets = BulletsHeap;
    //Buildings = BuildingsHeap;
    //Factories = FactoriesHeap;
    Houses = HousesHeap;
    //Infantry = InfantryHeap;
    //Overlays = OverlaysHeap;
    //Smudges = SmudgesHeap;
    //Templates = TemplatesHeap;
    //Terrains = TerrainsHeap;
    TeamTypes = TeamTypesHeap;
    //Teams = TeamsHeap;
    //Triggers = TriggersHeap;
    //Units = UnitsHeap;

    return true;
}
#pragma endregion

#pragma region SaveGame
void SaveGame::Read_Globals()
{
    Header.Read_Globals();
    ScenarioState.Read_Globals();

    GameMap = Map;

    // TODO: BaseClass
    // TODO: LogicClass
    // TODO: LayerClass
    // TODO: ScoreClass
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

    // TODO: Validate Layers, AiBase, Logic, Score

    return result;
}

bool SaveGame::Write_Globals() const
{
    if (!Validate()) {
        return false;
    }

    auto result = true;

    result = Header.Write_Globals() && ScenarioState.Write_Globals();

    Map = GameMap;


    // TODO: Write SaveGame Globals

    // TODO: Call Decode_Pointers after everything imported into Globals
    // TODO: Set PlayerPtr by getting house pointer for Header.PlayerHouseType enum value

    return result;
}

void SaveGame::Dump_Json(std::string& output) const
{
    const nlohmann::json save_json = *this;

    output = save_json.dump();
}
#pragma endregion
