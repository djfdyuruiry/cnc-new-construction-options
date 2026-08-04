#include "common/settings.h"

#include "function.h"
#include "tiberiandawnsettings.h"

void TiberianDawnSettings::Init(SettingsClass& common_settings)
{
    CommonSettings = &common_settings;
}

void TiberianDawnSettings::Load_MultiPlayer(INIClass& ini)
{
    CNC_LOGGER_DEBUG("Loading Tiberian Dawn multiplayer settings");

    auto& multiplayer_section = Get_Multiplayer_Section();

    multiplayer_section.With<IniRuleContext>(ini, [&](auto& c) {
        c.Load("Handle").With_Default("Noname")
         .Load("ScenarioNumber").With_Default(0).Then_Set(MPlayerScenarioNumber)
         .Load("BasesOn").With_Default(true).Then_Set(MPlayerBases)
         .Load("TiberiumRegrows").With_Default(true).template Then_Set_With_Type<bool>(MPlayerTiberium)
         .Load("CratesOn").With_Default(false).template Then_Set_With_Type<bool>(MPlayerGoodies)
         .Load("CaptureTheFlag").With_Default(false)
            .template With_Callback<bool>([] (auto ctf) { Special.IsCaptureTheFlag = ctf; })
         .template Load_With_Converter_Callback<PlayerColorType, TdTypeConverter>(
             "Color", REMAP_GOLD, [] (auto colour) { MPlayerPrefColor = colour; }
         )
         .template Load_With_Converter_Callback<HousesType, TdTypeConverter>(
             "Side", HOUSE_GOOD, [] (auto house) { MPlayerHouse = house; }
         )
        .Load("MaxScenarioNumber").With_Default(500);
    });

    //	Get the player's last-used Handle
    static constexpr auto max_name_length = std::size(MPlayerName);

    if (multiplayer_section.Get_C_Str("Handle", MPlayerName, std::size(MPlayerName)) >= max_name_length)
    {
        CNC_LOG_WARN(
            "'[MultiPlayer] -> Handle' in {} is too long. Maximum allowed characters: {}",
            IniFileName,
            max_name_length
        );
    }

    // build the multiplayer map descriptions collection
    const auto max_scenario_num = multiplayer_section.Get<int>("MaxScenarioNumber");

    MPlayerDescriptions.clear();

    for (auto i = 0; i < max_scenario_num; i++) {
        MPlayerDescriptions.emplace_back(std::make_unique<char[]>(256));
    }
}

void TiberianDawnSettings::Load(std::string ini_file_name, INIClass& ini)
{
    IniFileName = std::move(ini_file_name);

    // reset state
    CNC_LOGGER_INFO("Loading Tiberian Dawn settings from INI file: {}", IniFileName);

    CommonSettings->Load(IniFileName, ini);

    Get_Map_Section().With<IniRuleContext>(ini, [&](auto& c) {
        c.Load("PlacementDebugging").With_Default(false);
    });
    Load_MultiPlayer(ini);
}

bool TiberianDawnSettings::Placement_Debugging_Is_Enabled()
{
    return Get_Map_Section().Get<bool>("PlacementDebugging");
}

void TiberianDawnSettings::Update_MultiPlayer()
{
    CNC_LOGGER_DEBUG("Updating Tiberian Dawn multiplayer settings from globals variables");

    Get_Multiplayer_Section()
        .Set("Handle", MPlayerName)
        .Set("ScenarioNumber", MPlayerScenarioNumber)
        .Set("BasesOn", MPlayerBases)
        .Set("TiberiumRegrows", static_cast<bool>(MPlayerTiberium))
        .Set("CratesOn", static_cast<bool>(MPlayerGoodies))
        .Set("CaptureTheFlag", static_cast<bool>(Special.IsCaptureTheFlag))
        .Set_With_Converter<PlayerColorType, TdTypeConverter>("Color", static_cast<PlayerColorType>(MPlayerPrefColor))
        .Set_With_Converter<HousesType, TdTypeConverter>("Side", MPlayerHouse)
        .Set_Rule_Comment("MaxScenarioNumber", "load multiplayer scenarios up to this number, increase to load more")
        .Set("MaxScenarioNumber", static_cast<int>(MPlayerDescriptions.size()));
}

void TiberianDawnSettings::Update_Sections()
{
    Update_MultiPlayer();
}

void TiberianDawnSettings::Save(INIClass& ini)
{
    CNC_LOGGER_INFO("Saving Tiberian Dawn settings to INI file: {}", IniFileName);

    Update_Sections();

    CommonSettings->Save(ini);
}

RuleSections& TiberianDawnSettings::Get_Common_Sections()
{
    if (CommonSettings == nullptr) {
        throw std::runtime_error("Attempted to read common settings before TiberianDawnSettings::Init was called");
    }

    return CommonSettings->Get_Sections();
}

RuleSection& TiberianDawnSettings::Get_Map_Section()
{
    return Get_Common_Sections()[MapPlayerSection];
}

RuleSection& TiberianDawnSettings::Get_Multiplayer_Section()
{
    return Get_Common_Sections()[MultiPlayerSection];
}

TiberianDawnSettings TdSettings;
