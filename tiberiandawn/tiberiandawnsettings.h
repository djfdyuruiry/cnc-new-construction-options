#pragma once

#include <string>

#include "common/logger.h"
#include "common/rulesections.h"

class INIClass;

class TiberianDawnSettings
{
public:
    static constexpr auto MultiPlayerSection = "MultiPlayer";

    void Load(std::string ini_file_name, INIClass& ini);

    RuleSection& MultiPlayer() const;

    void Update_MultiPlayer() const;
    void Update() const;
    void Save(INIClass& ini) const;

    RuleSection& operator[](std::string_view section) const;

private:
    static inline const auto& Logger = CncLogger::For(TiberianDawnSettings);

    std::string IniFileName;
    RuleSections Settings;

    void Load_MultiPlayer(INIClass& ini);
};

extern TiberianDawnSettings TdSettings;
