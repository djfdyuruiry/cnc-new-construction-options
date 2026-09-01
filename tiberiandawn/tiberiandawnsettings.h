#pragma once

#include <string>

#include "common/logger.h"
#include "common/rulesections.h"
#include "common/settings.h"

class INIClass;

/**
 * Holds settings specific to the Tiberian Dawn game engine that are stored
 * in the CONQUER.INI file. Used by the multiplayer setup screen for Skirmish
 * and network play.
 *
 * Requires Settings singleton instance of SettingsClass for save/load from
 * INI. Instance pointer should be provided by calling the ::Init method.
 */
class TiberianDawnSettings
{
public:
    static constexpr auto EditorSection = "Editor";
    static constexpr auto MapPlayerSection = "Map";
    static constexpr auto MultiPlayerSection = "MultiPlayer";

    void Init(SettingsClass& common_settings);
    void Load(std::string ini_file_name, INIClass& ini);

    // game engine options
    bool Cash_Sounds_Enabled();

    // editor
    bool Enforce_OwnableBy_In_Editor();
    ColorType Get_Editor_Trigger_Color();
    ColorType Get_Editor_Waypoint_Color();
    bool Display_Object_Icons();

    // map
    bool Placement_Debugging_Is_Enabled();

    void Update_MultiPlayer();
    void Update_Sections();
    void Save(INIClass& ini);

private:
    static inline const auto& Logger = CncLogger::For(TiberianDawnSettings);

    SettingsClass* CommonSettings = nullptr;
    std::string IniFileName;

    RuleSections& Get_Common_Sections();
    RuleSection& Get_Editor_Section();
    RuleSection& Get_Map_Section();
    RuleSection& Get_Multiplayer_Section();

    void Load_MultiPlayer(INIClass& ini);
};

// singleton instance
extern TiberianDawnSettings TdSettings;
