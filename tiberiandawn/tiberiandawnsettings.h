#pragma once

#include <string>

#include "common/logger.h"
#include "common/rulesections.h"

class INIClass;

/**
 * Holds settings specific to the Tiberian Dawn game engine that are stored
 * in the CONQUER.INI file. Used by the multiplayer setup screen for Skirmish
 * and network play.
 */
class TiberianDawnSettings
{
public:
    static constexpr auto MultiPlayerSection = "MultiPlayer";

    void Load(std::string ini_file_name, INIClass& ini);

    const RuleSection& MultiPlayer() const;
    RuleSection& Editable_MultiPlayer();

    void Update_MultiPlayer();
    void Update();
    void Save(INIClass& ini) const;

    const RuleSection& operator[](std::string_view section) const;
    RuleSection& operator[](std::string_view section);

private:
    static inline const auto& Logger = CncLogger::For(TiberianDawnSettings);

    std::string IniFileName;
    RuleSections Settings;

    void Load_MultiPlayer(INIClass& ini);
};

extern TiberianDawnSettings TdSettings;
