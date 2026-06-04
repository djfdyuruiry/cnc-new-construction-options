#include "function.h"
#include "tiberiandawnsettings.h"

void TiberianDawnSettings::Load_MultiPlayer(INIClass& ini)
{
    CNC_LOGGER_DEBUG("Loading Tiberian Dawn multiplayer settings");

    auto& multiplayer_settings = Settings[MultiPlayerSection];

    multiplayer_settings.With<IniRuleContext>(ini, [&](auto& c) {
        c.Load("Handle").With_Default("Noname")
         .Load("ScenarioNumber").With_Default(0)
         .Load("BasesOn").With_Default(true)
         .Load("TiberiumRegrows").With_Default(true)
         .Load("CratesOn").With_Default(false)
         .Load("CaptureTheFlag").With_Default(false)
         .template Load_With_Converter<PlayerColorType, TdTypeConverter>("Color", REMAP_GOLD)
         .template Load_With_Converter<HousesType, TdTypeConverter>("Side", HOUSE_GOOD);
    });

    //	Get the player's last-used Handle
    static constexpr auto max_name_length = std::size(MPlayerName);

    if (multiplayer_settings.Get_C_Str("Handle", MPlayerName, std::size(MPlayerName)) >= max_name_length)
    {
        CNC_LOG_WARN(
            "'[MultiPlayer] -> Handle' in {} is too long. Maximum allowed characters: {}",
            IniFileName,
            max_name_length
        );
    }

    MPlayerScenarioNumber = multiplayer_settings.Get<int>("ScenarioNumber");
    MPlayerBases = multiplayer_settings.Get<bool>("BasesOn");
    MPlayerTiberium = multiplayer_settings.Get<bool>("TiberiumRegrows");
    MPlayerGoodies = multiplayer_settings.Get<bool>("CratesOn");
    Special.IsCaptureTheFlag = multiplayer_settings.Get<bool>("CaptureTheFlag");

    //	Get the player's last-used Color
    MPlayerPrefColor = multiplayer_settings.Get_With_Converter<PlayerColorType, TdTypeConverter>("Color");
    MPlayerHouse = multiplayer_settings.Get_With_Converter<HousesType, TdTypeConverter>("Side");
}

void TiberianDawnSettings::Load(std::string ini_file_name, INIClass& ini)
{
    // reset state
    IniFileName = std::move(ini_file_name);
    Settings = RuleSections();

    CNC_LOGGER_INFO("Loading Tiberian Dawn settings from INI file: {}", IniFileName);

    Load_MultiPlayer(ini);
}

void TiberianDawnSettings::Update_MultiPlayer() const
{
    CNC_LOGGER_DEBUG("Updating Tiberian Dawn multiplayer settings from globals variables");

    MultiPlayer().Set("Handle", MPlayerName)
        .Set("ScenarioNumber", MPlayerScenarioNumber)
        .Set("BasesOn", MPlayerBases)
        .Set("TiberiumRegrows", static_cast<bool>(MPlayerTiberium))
        .Set("CratesOn", static_cast<bool>(MPlayerGoodies))
        .Set("CaptureTheFlag", static_cast<bool>(Special.IsCaptureTheFlag))
        .Set_With_Converter<PlayerColorType, TdTypeConverter>("Color", static_cast<PlayerColorType>(MPlayerPrefColor))
        .Set_With_Converter<HousesType, TdTypeConverter>("Side", MPlayerHouse);
}

void TiberianDawnSettings::Update() const
{
    Update_MultiPlayer();
}

void TiberianDawnSettings::Save(INIClass& ini) const
{
    CNC_LOGGER_INFO("Saving Tiberian Dawn settings to INI file: {}", IniFileName);

    Settings.Save_All_To_Ini(ini);
}

RuleSection& TiberianDawnSettings::MultiPlayer() const
{
    if (!Settings.Has_Section(MultiPlayerSection)) {
        throw std::runtime_error("Attempted to get multiplayer settings before TdSettings.Load(...) was called");
    }

    return Settings[MultiPlayerSection];
}

RuleSection& TiberianDawnSettings::operator[](std::string_view section) const
{
    return Settings[section];
}

TiberianDawnSettings TdSettings;
