//
// Copyright 2020 Electronic Arts Inc.
//
// TiberianDawn.DLL and RedAlert.dll and corresponding source code is free
// software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.

// TiberianDawn.DLL and RedAlert.dll and corresponding source code is distributed
// in the hope that it will be useful, but with permitted additional restrictions
// under Section 7 of the GPL. See the GNU General Public License in LICENSE.TXT
// distributed with this program. You should have received a copy of the
// GNU General Public License along with permitted additional restrictions
// with this program. If not, see https://github.com/electronicarts/CnC_Remastered_Collection

/* $Header: /CounterStrike/RULES.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : RULES.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 05/12/96                                                     *
 *                                                                                             *
 *                  Last Update : September 10, 1996 [JLB]                                     *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   DifficultyClass::DifficultyClass -- Default constructor for difficulty class object.      *
 *   RulesClass::RulesClass -- Default constructor for rules class object.                     *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include <algorithm>

#include "common/logger.h"
#include "common/rulesections.h"

#include "function.h"
#include "ccini.h"

/***********************************************************************************************
 * DifficultyClass::DifficultyClass -- Default constructor for difficulty class object.        *
 *                                                                                             *
 *    This is the default constructor for the difficulty class object. Although it initializes *
 *    the rule data with default values, it is expected that they will all be overridden by    *
 *    the rules control file.                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/18/2019 SKY : Created.                                                                 *
 *=============================================================================================*/
DifficultyClass::DifficultyClass(void)
    : FirepowerBias(1)
    , GroundspeedBias(1)
    , AirspeedBias(1)
    , ArmorBias(1)
    , ROFBias(1)
    , CostBias(1)
    , BuildSpeedBias(1)
    , RepairDelay(0.02f)
    , BuildDelay(0.03f)
    , IsBuildSlowdown(false)
    , IsWallDestroyer(true)
    , IsContentScan(false)
{
}

/***********************************************************************************************
 * RulesClass::RulesClass -- Default constructor for rules class object.                       *
 *                                                                                             *
 *    This is the default constructor for the rules class object. Although it initializes the  *
 *    rule data with default values, it is expected that they will all be overridden by the    *
 *    rules control file.                                                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/17/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
RulesClass::RulesClass(void)
    : AttackInterval(3)
    , AttackDelay(5)
    , PowerEmergencyFraction(3, 4)
    , HelipadRatio(".12")
    , HelipadLimit(6)
    , TeslaRatio(".8")
    , TeslaLimit(5)
    , AARatio(".14")
    , AALimit(10)
    , DefenseRatio(".5")
    , DefenseLimit(25)
    , WarRatio(".1")
    , WarLimit(3)
    , BarracksRatio(".16")
    , BarracksLimit(2)
    , RefineryLimit(7)
    , RefineryRatio(".18")
    , BaseSizeAdd(3)
    , PowerSurplus(50)
    , MaxIQ(5)
    , IQSuperWeapons(4)
    , IQProduction(5)
    , IQGuardArea(4)
    , IQRepairSell(1)
    , IQCrush(2)
    , IQScatter(3)
    , IQContentScan(4)
    , IQAircraft(4)
    , IQHarvester(2)
    , IQSellBack(2)
    , InfantryReserve(3000)
    , InfantryBaseMult(1)
    , IsComputerParanoid(false)
    , IsCompEasyBonus(false)
    , IsFineDifficulty(false)
    , AllowSuperWeapons(true)
{
#ifndef REMASTER_BUILD

    /* giulianob: Bump FirepowerBias just a little, else the enemy do too much damage. */
    Diff[DIFF_EASY].FirepowerBias = "1.1";
    /* giulianob: Same thing with speed. */
    Diff[DIFF_EASY].GroundspeedBias = "1.1";
    Diff[DIFF_EASY].AirspeedBias = "1.1";
    /* giulianob: Don't bump the ArmorBias, else enemy units on hard becomes
       undestroyable.  */
    Diff[DIFF_EASY].ArmorBias = 1;
    Diff[DIFF_EASY].ROFBias = "0.8";
    Diff[DIFF_EASY].CostBias = "0.8";
    Diff[DIFF_EASY].BuildSpeedBias = "0.6";
    Diff[DIFF_EASY].RepairDelay = "0.001";
    Diff[DIFF_EASY].BuildDelay = "0.002";
    Diff[DIFF_EASY].IsBuildSlowdown = false;
    Diff[DIFF_EASY].IsWallDestroyer = true;
    Diff[DIFF_EASY].IsContentScan = true;

    Diff[DIFF_NORMAL].FirepowerBias = 1;
    Diff[DIFF_NORMAL].GroundspeedBias = 1;
    Diff[DIFF_NORMAL].AirspeedBias = 1;
    Diff[DIFF_NORMAL].ArmorBias = 1;
    Diff[DIFF_NORMAL].ROFBias = 1;
    Diff[DIFF_NORMAL].CostBias = 1;
    Diff[DIFF_NORMAL].BuildSpeedBias = 1;
    Diff[DIFF_NORMAL].RepairDelay = "0.02";
    Diff[DIFF_NORMAL].BuildDelay = "0.03";
    Diff[DIFF_NORMAL].IsBuildSlowdown = true;
    Diff[DIFF_NORMAL].IsWallDestroyer = true;
    Diff[DIFF_NORMAL].IsContentScan = true;

    Diff[DIFF_HARD].FirepowerBias = "0.9";
    Diff[DIFF_HARD].GroundspeedBias = "0.9";
    Diff[DIFF_HARD].AirspeedBias = "0.9";
    Diff[DIFF_HARD].ArmorBias = "1.05";
    Diff[DIFF_HARD].ROFBias = "1.05";
    Diff[DIFF_HARD].CostBias = 1;
    Diff[DIFF_HARD].BuildSpeedBias = 1;
    Diff[DIFF_HARD].RepairDelay = "0.05";
    Diff[DIFF_HARD].BuildDelay = "0.1";
    Diff[DIFF_HARD].IsBuildSlowdown = true;
    Diff[DIFF_HARD].IsWallDestroyer = true;
    Diff[DIFF_HARD].IsContentScan = true;
#endif
}

/***********************************************************************************************
 * Difficulty_Get -- Fetch the difficulty bias values.                                         *
 *                                                                                             *
 *    This will fetch the difficulty bias values for the section specified.                    *
 *                                                                                             *
 * INPUT:   ini   -- Reference the INI database to fetch the values from.                      *
 *                                                                                             *
 *          diff  -- Reference to the difficulty class object to fill in with the values.      *
 *                                                                                             *
 *          section  -- The section identifier to lift the values from.                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/11/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static void Difficulty_Get(CCINIClass& ini, DifficultyClass& diff, char const* section)
{
    if (ini.Is_Present(section)) {
        diff.FirepowerBias = ini.Get_Fixed(section, "FirePower", diff.FirepowerBias);
        diff.GroundspeedBias = ini.Get_Fixed(section, "Groundspeed", diff.GroundspeedBias);
        diff.AirspeedBias = ini.Get_Fixed(section, "Airspeed", diff.AirspeedBias);
        diff.ArmorBias = ini.Get_Fixed(section, "Armor", diff.ArmorBias);
        diff.ROFBias = ini.Get_Fixed(section, "ROF", diff.ROFBias);
        diff.CostBias = ini.Get_Fixed(section, "Cost", diff.CostBias);
        diff.RepairDelay = ini.Get_Fixed(section, "RepairDelay", diff.RepairDelay);
        diff.BuildDelay = ini.Get_Fixed(section, "BuildDelay", diff.BuildDelay);
        diff.IsBuildSlowdown = ini.Get_Bool(section, "BuildSlowdown", diff.IsBuildSlowdown);
        diff.BuildSpeedBias = ini.Get_Fixed(section, "BuildTime", diff.BuildSpeedBias);
        diff.IsWallDestroyer = ini.Get_Bool(section, "DestroyWalls", diff.IsWallDestroyer);
        diff.IsContentScan = ini.Get_Bool(section, "ContentScan", diff.IsContentScan);
    }
}

/***********************************************************************************************
 * Difficulty_Put -- Fetch the difficulty bias values.                                         *
 *                                                                                             *
 *    This will fetch the difficulty bias values for the section specified.                    *
 *                                                                                             *
 * INPUT:   ini   -- Reference the INI database to fetch the values from.                      *
 *                                                                                             *
 *          diff  -- Reference to the difficulty class object to fill in with the values.      *
 *                                                                                             *
 *          section  -- The section identifier to lift the values from.                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/11/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static void Difficulty_Put(CCINIClass& ini, const DifficultyClass& diff, char const* section)
{
    ini.Put_Fixed(section, "FirePower", diff.FirepowerBias);
    ini.Put_Fixed(section, "Groundspeed", diff.GroundspeedBias);
    ini.Put_Fixed(section, "Airspeed", diff.AirspeedBias);
    ini.Put_Fixed(section, "Armor", diff.ArmorBias);
    ini.Put_Fixed(section, "ROF", diff.ROFBias);
    ini.Put_Fixed(section, "Cost", diff.CostBias);
    ini.Put_Fixed(section, "RepairDelay", diff.RepairDelay);
    ini.Put_Fixed(section, "BuildDelay", diff.BuildDelay);
    ini.Put_Bool(section, "BuildSlowdown", diff.IsBuildSlowdown);
    ini.Put_Fixed(section, "BuildTime", diff.BuildSpeedBias);
    ini.Put_Bool(section, "DestroyWalls", diff.IsWallDestroyer);
    ini.Put_Bool(section, "ContentScan", diff.IsContentScan);
}

/***********************************************************************************************
 * RulesClass::Process -- Fetch the bulk of the rule data from the control file.               *
 *                                                                                             *
 *    This routine will fetch the rule data from the control file.                             *
 *                                                                                             *
 * INPUT:   file  -- Reference to the rule file to process.                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/17/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void RulesClass::Init(CCINIClass& ini)
{
    AI(ini);
    IQ(ini);
    Difficulty(ini);

    Init_Sections(ini);
    Apply_Static_And_Global_Values();
}

void RulesClass::Init_For_Scenario(
    const ScenarioClass& scenario,
    const GameType& game_to_play,
    const SpecialClass special_options,
    const std::optional<bool> superweapons_allowed
)
{
    const std::string scenario_ini_file = scenario.FileName;

    if (CncStringUtils::Is_Blank(scenario_ini_file)) {
        CNC_LOGGER_WARN("Not loading rules for scenario - no INI filename provided");
        return;
    }

    CCFileClass ini_file(scenario_ini_file.c_str());

    if (!ini_file.Is_Available()) {
        CNC_LOGGER_WARN("Not loading rules for scenario - INI file is missing: {}", scenario_ini_file);
        return;
    }

    CCINIClass ini;

    if (!ini.Load(ini_file, true)) {
        CNC_LOGGER_FATAL("Failed to load rules for scenario - INI file is corrupt: {}", scenario_ini_file);
        return;
    }

    CNC_LOGGER_INFO("Reading scenario rules from INI file: {}", scenario_ini_file);

    Init(ini);
    Init_Types(ini);

    // ensure we restore any skirmish game options after reading rules
    if (game_to_play == GAME_SKIRMISH || game_to_play == GAME_GLYPHX_MULTIPLAYER) {
        special_options.Write_Rules(Sections);
        AllowSuperWeapons = superweapons_allowed.value_or(AllowSuperWeapons);
    }
}

/**
 * Purge all existing rule definitions, leaving the rule sections
 * and type rules blank.
 */
void RulesClass::Reset()
{
    Sections = RuleSections();

    static const auto type_names = std::vector {
        TdTypeConverter::Get_Type_Name<AnimType>(),
        TdTypeConverter::Get_Type_Name<WarheadType>(),
        TdTypeConverter::Get_Type_Name<BulletType>(),
        TdTypeConverter::Get_Type_Name<WeaponType>(),
        TdTypeConverter::Get_Type_Name<AircraftType>(),
        TdTypeConverter::Get_Type_Name<StructType>(),
        TdTypeConverter::Get_Type_Name<InfantryType>(),
        TdTypeConverter::Get_Type_Name<UnitType>(),
        TdTypeConverter::Get_Type_Name<HousesType>(),
    };

    TypeRules.clear();

    for (const auto& type_name : type_names) {
        TypeRules[type_name] = std::make_unique<RuleSections>();
    }
}

/**
 * Init rules from INI files, falls back to hardcoded values for any missing INI file.
 *
 * Also generates any missing INI file, using defaults from hardcoded values.
 */
void RulesClass::Init()
{
    Reset();

    CCFileClass ini_file(RulesFilename);
    CCINIClass ini;

    // Prevent an embedded ::RulesFilename file shipped with some
    // C&C mix files being preferred over a standalone file.
    ini_file.DisableMixFileSearching();

    const auto ini_file_exists = ini_file.Is_Available();

    if (ini_file_exists) {
        if (!ini.Load(ini_file, false)) {
            CNC_LOGGER_FATAL("Failed to rules from existing file - INI file is corrupt: {}", RulesFilename);
            return;
        }

        CNC_LOGGER_INFO("Loaded rules from existing file: {}", RulesFilename);
    }

    Init(ini);
    Init_Types();

    // provide player with a default ::RulesFilename file
    if (!ini_file_exists) {
        CNC_LOGGER_INFO("Writing rules to new file: {}", RulesFilename);
        Export(ini);

        if (!ini.Save(ini_file, false)) {
            CNC_LOGGER_FATAL("Failed to generate rules file: {}", RulesFilename);
        }
    }

    ini_file.Close();
}

/***********************************************************************************************
 * RulesClass::Process -- Fetch the bulk of the rule data from the control file.               *
 *                                                                                             *
 *    This routine will fetch the rule data from the control file.                             *
 *                                                                                             *
 * INPUT:   file  -- Reference to the rule file to process.                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/17/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void RulesClass::Export(CCINIClass& ini) const
{
    Export_AI(ini);
    Export_IQ(ini);
    Export_Difficulty(ini);
    Sections.Save_All_To_Ini(ini);
}

/***********************************************************************************************
 * RulesClass::AI -- Processes the AI control constants from the database.                     *
 *                                                                                             *
 *    This will examine the database specified and set the AI override values accordingly.     *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database that holds the AI overrides.                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void RulesClass::AI(CCINIClass& ini)
{
    static constexpr char AI[] = "AI";

    if (!ini.Is_Present(AI)) {
        return;
    }

    AttackInterval = ini.Get_Fixed(AI, "AttackInterval", AttackInterval);
    AttackDelay = ini.Get_Fixed(AI, "AttackDelay", AttackDelay);
    InfantryReserve = ini.Get_Int(AI, "InfantryReserve", InfantryReserve);
    InfantryBaseMult = ini.Get_Int(AI, "InfantryBaseMult", InfantryBaseMult);
    PowerSurplus = ini.Get_Int(AI, "PowerSurplus", PowerSurplus);
    BaseSizeAdd = ini.Get_Int(AI, "BaseSizeAdd", BaseSizeAdd);
    RefineryRatio = ini.Get_Fixed(AI, "RefineryRatio", RefineryRatio);
    RefineryLimit = ini.Get_Int(AI, "RefineryLimit", RefineryLimit);
    BarracksRatio = ini.Get_Fixed(AI, "BarracksRatio", BarracksRatio);
    BarracksLimit = ini.Get_Int(AI, "BarracksLimit", BarracksLimit);
    WarRatio = ini.Get_Fixed(AI, "WarRatio", WarRatio);
    WarLimit = ini.Get_Int(AI, "WarLimit", WarLimit);
    DefenseRatio = ini.Get_Fixed(AI, "DefenseRatio", DefenseRatio);
    DefenseLimit = ini.Get_Int(AI, "DefenseLimit", DefenseLimit);
    AARatio = ini.Get_Fixed(AI, "AARatio", AARatio);
    AALimit = ini.Get_Int(AI, "AALimit", AALimit);
    TeslaRatio = ini.Get_Fixed(AI, "ObeliskRatio", TeslaRatio);
    TeslaLimit = ini.Get_Int(AI, "ObeliskLimit", TeslaLimit);
    HelipadRatio = ini.Get_Fixed(AI, "HelipadRatio", HelipadRatio);
    HelipadLimit = ini.Get_Int(AI, "HelipadLimit", HelipadLimit);
    IsCompEasyBonus = ini.Get_Bool(AI, "CompEasyBonus", IsCompEasyBonus);
    IsComputerParanoid = ini.Get_Bool(AI, "Paranoid", IsComputerParanoid);
    PowerEmergencyFraction = ini.Get_Fixed(AI, "PowerEmergency", PowerEmergencyFraction);
}

/***********************************************************************************************
 * RulesClass::Export_AI -- Processes the AI control constants to the database.                *
 *                                                                                             *
 *    This will examine the database specified and set the AI override values accordingly.     *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database that holds the AI overrides.                *
 *                                                                                             *
 * OUTPUT:  bool; Was the AI section found and processed?                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void RulesClass::Export_AI(CCINIClass& ini) const
{
    static constexpr char AI[] = "AI";

    ini.Put_Fixed(AI, "AttackInterval", AttackInterval);
    ini.Put_Fixed(AI, "AttackDelay", AttackDelay);
    ini.Put_Int(AI, "InfantryReserve", InfantryReserve);
    ini.Put_Int(AI, "InfantryBaseMult", InfantryBaseMult);
    ini.Put_Int(AI, "PowerSurplus", PowerSurplus);
    ini.Put_Int(AI, "BaseSizeAdd", BaseSizeAdd);
    ini.Put_Fixed(AI, "RefineryRatio", RefineryRatio);
    ini.Put_Int(AI, "RefineryLimit", RefineryLimit);
    ini.Put_Fixed(AI, "BarracksRatio", BarracksRatio);
    ini.Put_Int(AI, "BarracksLimit", BarracksLimit);
    ini.Put_Fixed(AI, "WarRatio", WarRatio);
    ini.Put_Int(AI, "WarLimit", WarLimit);
    ini.Put_Fixed(AI, "DefenseRatio", DefenseRatio);
    ini.Put_Int(AI, "DefenseLimit", DefenseLimit);
    ini.Put_Fixed(AI, "AARatio", AARatio);
    ini.Put_Int(AI, "AALimit", AALimit);
    ini.Put_Fixed(AI, "ObeliskRatio", TeslaRatio);
    ini.Put_Int(AI, "ObeliskLimit", TeslaLimit);
    ini.Put_Fixed(AI, "HelipadRatio", HelipadRatio);
    ini.Put_Int(AI, "HelipadLimit", HelipadLimit);
    ini.Put_Bool(AI, "CompEasyBonus", IsCompEasyBonus);
    ini.Put_Bool(AI, "Paranoid", IsComputerParanoid);
    ini.Put_Fixed(AI, "PowerEmergency", PowerEmergencyFraction);
}

/***********************************************************************************************
 * RulesClass::IQ -- Fetches the IQ control values from the INI database.                      *
 *                                                                                             *
 *    This will scan the database specified and retrieve the IQ control values from it. These  *
 *    IQ control values are what gives the IQ rating meaning. It fundimentally controls how    *
 *    the computer behaves.                                                                    *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database to read the IQ controls from.               *
 *                                                                                             *
 * OUTPUT:  bool; Was the IQ section found and processed?                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void RulesClass::IQ(CCINIClass& ini)
{
    static constexpr char IQCONTROL[] = "IQ";

    if (!ini.Is_Present(IQCONTROL)) {
        return;
    }

    MaxIQ = ini.Get_Int(IQCONTROL, "MaxIQLevels", MaxIQ);
    IQSuperWeapons = ini.Get_Int(IQCONTROL, "SuperWeapons", IQSuperWeapons);
    IQProduction = ini.Get_Int(IQCONTROL, "Production", IQProduction);
    IQGuardArea = ini.Get_Int(IQCONTROL, "GuardArea", IQGuardArea);
    IQRepairSell = ini.Get_Int(IQCONTROL, "RepairSell", IQRepairSell);
    IQCrush = ini.Get_Int(IQCONTROL, "AutoCrush", IQCrush);
    IQScatter = ini.Get_Int(IQCONTROL, "Scatter", IQScatter);
    IQContentScan = ini.Get_Int(IQCONTROL, "ContentScan", IQContentScan);
    IQAircraft = ini.Get_Int(IQCONTROL, "Aircraft", IQAircraft);
    IQHarvester = ini.Get_Int(IQCONTROL, "Harvester", IQHarvester);
    IQSellBack = ini.Get_Int(IQCONTROL, "SellBack", IQSellBack);
}

/***********************************************************************************************
 * RulesClass::Export_IQ -- Exports the IQ control values from the INI database.               *
 *                                                                                             *
 *    This will scan the database specified and retrieve the IQ control values from it. These  *
 *    IQ control values are what gives the IQ rating meaning. It fundimentally controls how    *
 *    the computer behaves.                                                                    *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database to read the IQ controls from.               *
 *                                                                                             *
 * OUTPUT:  bool; Was the IQ section found and processed?                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void RulesClass::Export_IQ(CCINIClass& ini) const
{
    static constexpr char IQCONTROL[] = "IQ";

    ini.Put_Int(IQCONTROL, "MaxIQLevels", MaxIQ);
    ini.Put_Int(IQCONTROL, "SuperWeapons", IQSuperWeapons);
    ini.Put_Int(IQCONTROL, "Production", IQProduction);
    ini.Put_Int(IQCONTROL, "GuardArea", IQGuardArea);
    ini.Put_Int(IQCONTROL, "RepairSell", IQRepairSell);
    ini.Put_Int(IQCONTROL, "AutoCrush", IQCrush);
    ini.Put_Int(IQCONTROL, "Scatter", IQScatter);
    ini.Put_Int(IQCONTROL, "ContentScan", IQContentScan);
    ini.Put_Int(IQCONTROL, "Aircraft", IQAircraft);
    ini.Put_Int(IQCONTROL, "Harvester", IQHarvester);
    ini.Put_Int(IQCONTROL, "SellBack", IQSellBack);
}

/***********************************************************************************************
 * RulesClass::Difficulty -- Fetch the various difficulty group settings.                      *
 *                                                                                             *
 *    This routine is used to fetch the various group settings for the difficulty levels.      *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database that has the difficulty setting values.     *
 *                                                                                             *
 * OUTPUT:  bool; Was the difficulty section found and processed.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/10/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void RulesClass::Difficulty(CCINIClass& ini)
{
#ifndef REMASTER_BUILD
    Difficulty_Get(ini, Diff[DIFF_EASY], "Easy");
    Difficulty_Get(ini, Diff[DIFF_NORMAL], "Normal");
    Difficulty_Get(ini, Diff[DIFF_HARD], "Difficult");
#endif
}

/**
 * Load type instance rules from a given INI context, falling back to hardcoded
 * defaults from C++ code. The rules section for the given type is completely
 * reset by this call; existing changes to C++ instances or rules are lost.
 *
 * This function also sets up an event handler to update instance properties if
 * the corresponding rules change at runtime. This is primarily used to allow the
 * Lua APIs to interact with the classes indirectly via the Rules section.
 */
template<EnumSignedChar U, RulesTypeClass<U> T>
static RuleSections& Init_Type(RulesClass& rules, U first, U count, CCINIClass& ini, const CncLogger& Logger)
{
    // get name for type and rules section
    auto type_name = TdTypeConverter::Get_Type_Name<U>();

    CNC_LOGGER_INFO("Processing rule sections for type: {}", type_name);

    auto& sections = *rules.TypeRules[type_name];

    // override rules to ensure INI comments for all types reveal real names
    const auto old_is_named = Special.IsNamed;
    Special.IsNamed = true;

    for (auto i = first; i < count; ++i) {
        auto& typeInstance = T::As_Mutable_Reference(i);
        auto name = std::string(typeInstance.Name());

        // load type instance properties using INI
        sections.Add_Section(name, [&](auto& section, auto rule, const auto& value) {
            // trigger type instance properties update if rules cache is updated
            typeInstance.Read_Rules(section); // TODO: consider optimising this to only update the affected property
        })
        .Set_Ini_Comment(ini, std::string(Text_String(typeInstance.Full_Name())))
        .template Set_Converter_Section_Type<U, TdTypeConverter>()
        .template With<IniRuleContext>(ini, [&](auto& c) {
            // load initial values from INI, falling back to type instance hardcoded values
            typeInstance.Read_INI(c);
        });
    }

    // reset temporary rules override
    Special.IsNamed = old_is_named;

    return sections;
}

template<EnumSignedChar U, RulesTypeClass<U> T>
static void Init_Type(RulesClass& rules, U first, U count, const CncLogger& Logger)
{
    auto prefix = TdTypeConverter::Get_Type_Name<U>();
    const auto rules_filename = std::format("{}.INI", prefix);

    CCFileClass ini_file(rules_filename.c_str());
    CCINIClass ini;
    const auto ini_file_exists = ini_file.Is_Available();

    if (ini_file_exists) {
        CNC_LOGGER_DEBUG("Loading type rules from INI file: {}", rules_filename);

        if (!ini.Load(ini_file, false)) {
            CNC_LOGGER_FATAL("Failed to load type rules - INI file is corrupt: {}", rules_filename);
        }
    }

    auto& sections = Init_Type<U, T>(rules, first, count, ini, Logger);

    // TODO: Load new types instances for ini sections with names not found in game engine
    //       (needs type ptr/reference rework and enums refactoring - to allow types to grow rather than be static)

    // provide player with a default <PREFIX>.INI file
    if (!ini_file_exists) {
        CNC_LOGGER_DEBUG("Writing type rules to INI file: {}", rules_filename);

        sections.Save_All_To_Ini(ini);

        if (!ini.Save(ini_file, false)) {
            CNC_LOGGER_FATAL("Failed to save type rules to INI file: {}", rules_filename);
        }
    }

    ini_file.Close();
}

void RulesClass::Init_Types()
{
    // TODO: Add existing subclasses of ObjectTypeClass Overlay, Smudge, Template and Terrain
    Init_Type<AnimType, AnimTypeClass>(*this, ANIM_FIRST, ANIM_COUNT, Logger);
    Init_Type<WarheadType, WarheadTypeClass>(*this, WARHEAD_FIRST, WARHEAD_COUNT, Logger);
    Init_Type<BulletType, BulletTypeClass>(*this, BULLET_FIRST, BULLET_COUNT, Logger);
    Init_Type<WeaponType, WeaponTypeClass>(*this, WEAPON_FIRST, WEAPON_COUNT, Logger);
    Init_Type<AircraftType, AircraftTypeClass>(*this, AIRCRAFT_FIRST, AIRCRAFT_COUNT, Logger);
    Init_Type<StructType, BuildingTypeClass>(*this, STRUCT_FIRST, STRUCT_COUNT, Logger);
    Init_Type<InfantryType, InfantryTypeClass>(*this, INFANTRY_FIRST, INFANTRY_COUNT, Logger);
    Init_Type<UnitType, UnitTypeClass>(*this, UNIT_FIRST, UNIT_COUNT, Logger);
    Init_Type<HousesType, HouseTypeClass>(*this, HOUSE_FIRST, HOUSE_COUNT, Logger);
}

void RulesClass::Init_Types(CCINIClass& ini)
{
    // TODO: Add existing subclasses of ObjectTypeClass Overlay, Smudge, Template and Terrain
    Init_Type<AnimType, AnimTypeClass>(*this, ANIM_FIRST, ANIM_COUNT, ini, Logger);
    Init_Type<WarheadType, WarheadTypeClass>(*this, WARHEAD_FIRST, WARHEAD_COUNT, ini, Logger);
    Init_Type<BulletType, BulletTypeClass>(*this, BULLET_FIRST, BULLET_COUNT, ini, Logger);
    Init_Type<WeaponType, WeaponTypeClass>(*this, WEAPON_FIRST, WEAPON_COUNT, ini, Logger);
    Init_Type<AircraftType, AircraftTypeClass>(*this, AIRCRAFT_FIRST, AIRCRAFT_COUNT, ini, Logger);
    Init_Type<StructType, BuildingTypeClass>(*this, STRUCT_FIRST, STRUCT_COUNT, ini, Logger);
    Init_Type<InfantryType, InfantryTypeClass>(*this, INFANTRY_FIRST, INFANTRY_COUNT, ini, Logger);
    Init_Type<UnitType, UnitTypeClass>(*this, UNIT_FIRST, UNIT_COUNT, ini, Logger);
    Init_Type<HousesType, HouseTypeClass>(*this, HOUSE_FIRST, HOUSE_COUNT, ini, Logger);
}

/***********************************************************************************************
 * RulesClass::Export_Difficulty -- Export the various difficulty group settings.              *
 *                                                                                             *
 *    This routine is used to fetch the various group settings for the difficulty levels.      *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database that has the difficulty setting values.     *
 *                                                                                             *
 * OUTPUT:  bool; Was the difficulty section found and processed.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/10/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void RulesClass::Export_Difficulty(CCINIClass& ini) const
{
#ifndef REMASTER_BUILD
    Difficulty_Put(ini, Diff[DIFF_EASY], "Easy");
    Difficulty_Put(ini, Diff[DIFF_NORMAL], "Normal");
    Difficulty_Put(ini, Diff[DIFF_HARD], "Difficult");
#endif
}

void RulesClass::Assert_Section_Not_Present(std::string_view name)
{
    if (
        Sections.Has_Section(name) ||
        std::ranges::any_of(TypeRules, [&](const auto& s) { return s.second->Has_Section(name); })
    ) {
        CNC_LOGGER_FATAL(
            "An attempt was made to init a rules section twice, this is likely due to using a INI Name "
            "more than once in {} or for a type INI name (Infantry, Unit etc.). All INI names must be unique. "
            "INI Name: {}",
            RulesFilename,
            name
        );
    }
}
