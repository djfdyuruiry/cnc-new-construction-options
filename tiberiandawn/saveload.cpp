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

/* $Header:   F:\projects\c&c\vcs\code\saveload.cpv   2.18   16 Oct 1995 16:48:44   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  W E S T W O O D   S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SAVELOAD.CPP                                                 *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : August 23, 1994                                              *
 *                                                                                             *
 *                  Last Update : June 24, 1995 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Code_All_Pointers -- Code all pointers.                                                   *
 *   Decode_All_Pointers -- Decodes all pointers.                                              *
 *   Get_Savefile_Info -- gets description, scenario #, house                                  *
 *   Load_Game -- loads a saved game                                                           *
 *   Load_Misc_Values -- Loads miscellaneous variables.                                        *
 *   Load_Misc_Values -- loads miscellaneous variables                                         *
 *   Read_Object -- reads an object from disk, in a safe way                                   *
 *   Save_Game -- saves a game to disk                                                         *
 *   Save_Misc_Values -- saves miscellaneous variables                                         *
 *   Target_To_TechnoType -- converts TARGET to TechnoTypeClass                                *
 *   TechnoType_To_Target -- converts TechnoTypeClass to TARGET                                *
 *   Write_Object -- reads an object from disk, in a safe way                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "function.h"
#include "savegame.h"

extern bool DLLSave(FileClass& file);
extern bool DLLLoad(FileClass& file);

/*
********************************** Defines **********************************
*/
#define SAVEGAME_VERSION                                                                                               \
    (DESCRIP_MAX + 0x01000003                                                                                          \
     + (sizeof(AircraftClass) + sizeof(AircraftTypeClass) + sizeof(AnimClass) + sizeof(AnimTypeClass)                  \
        + sizeof(BuildingClass) + sizeof(BuildingTypeClass) + sizeof(BulletClass) + sizeof(BulletTypeClass)            \
        + sizeof(HouseClass) + sizeof(HouseTypeClass) + sizeof(InfantryClass) + sizeof(InfantryTypeClass)              \
        + sizeof(OverlayClass) + sizeof(OverlayTypeClass) + sizeof(SmudgeClass) + sizeof(SmudgeTypeClass)              \
        + sizeof(TeamClass) + sizeof(TeamTypeClass) + sizeof(TemplateClass) + sizeof(TemplateTypeClass)                \
        + sizeof(TerrainClass) + sizeof(TerrainTypeClass) + sizeof(UnitClass) + sizeof(UnitTypeClass)                  \
        + sizeof(MouseClass) + sizeof(CellClass) + sizeof(FactoryClass) + sizeof(BaseClass) + sizeof(LayerClass)       \
        + sizeof(Scen.BriefingText) + sizeof(Scen.Waypoint) + sizeof(Scen.FileName)))

/***************************************************************************
 * Save_Game -- saves a game to disk                                       *
 *                                                                         *
 * Saving the Map:                                                         *
 *     DisplayClass::Save() invokes CellClass's Write() for every cell     *
 *     that needs to be saved.  A cell needs to be saved if it contains    *
 *     any special data at all, such as a TIcon, or an Occupier.           *
 *   The cell saves its own CellTrigger pointer, converted to a TARGET.    *
 *                                                                         *
 * Saving game objects:                                                    *
 *   - Any object stored in an ArrayOf class needs to be saved.  The ArrayOf*
 *     Save() routine invokes each object's Write() routine, if that       *
 *     object's IsActive is set.                                           *
 *                                                                         *
 * Saving the layers:                                                      *
 *   The Map's Layers (Ground, Air, etc) of things that are on the map,    *
 *     and the Logic's Layer of things to process both need to be saved.   *
 *     LayerClass::Save() writes the entire layer array to disk            *
 *                                                                         *
 * Saving the houses:                                                      *
 *   Each house needs to be saved, to record its Credits, Power, etc.      *
 *                                                                         *
 * Saving miscellaneous data:                                              *
 *   There are a lot of miscellaneous variables to save, such as the       *
 *     map's dimensions, the player's house, etc.                          *
 *                                                                         *
 * INPUT:                                                                  *
 *      id      numerical ID, for the file extension                       *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      true = OK, false = error                                           *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/28/1994 BR : Created.                                              *
 *=========================================================================*/
bool Save_Game(int id, char* descr)
{
    char name[_MAX_FNAME + _MAX_EXT];

    /*
    **	Generate the filename to save
    */
    sprintf(name, "SAVEGAME.%03d", id);

    return Save_Game(name, descr);
}

/*
** Version that takes file name. ST - 9/9/2019 11:10AM
*/
bool Save_Game(const char* file_name, const char* descr)
{
    CDFileClass save_file;

    if (!save_file.Open(file_name, WRITE)) {
        return false;
    }

    SaveGame save;
    std::string save_json;

    save.Header.Version = "1.0";
    static constexpr char ctrlZ = 26;
    save.Header.Description = std::format("{}\r\n{}", descr, ctrlZ);

    save.Read_Globals();

    save.Dump_Json(save_json);

    save_file.Write(save_json);
    save_file.Close();

    return (true);
}

/***************************************************************************
 * Load_Game -- loads a saved game                                         *
 *                                                                         *
 * This routine loads the data in the same way it was saved out.           *
 *                                                                         *
 * Loading the Map:                                                        *
 *   - DisplayClass::Load() invokes CellClass's Load() for every cell      *
 *     that was saved.                                                     *
 * - The cell loads its own CellTrigger pointer.                           *
 *                                                                         *
 * Loading game objects:                                                   *
 * - IHeap's Load() routine loads the # of objects stored, and loads       *
 *   each object.                                                          *
 * - Triggers: Add themselves to the HouseTriggers if they're associated   *
 *   with a house                                                          *
 *                                                                         *
 * Loading the layers:                                                     *
 *     LayerClass::Load() reads the entire layer array to disk             *
 *                                                                         *
 * Loading the houses:                                                     *
 *   Each house is loaded in its entirety.                                 *
 *                                                                         *
 * Loading miscellaneous data:                                             *
 *   There are a lot of miscellaneous variables to load, such as the       *
 *     map's dimensions, the player's house, etc.                          *
 *                                                                         *
 * INPUT:                                                                  *
 *      id         numerical ID, for the file extension                    *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      true = OK, false = error                                           *
 *                                                                         *
 * WARNINGS:                                                               *
 *      If this routine returns false, the entire game will be in an       *
 *      unknown state, so the scenario will have to be re-initialized.     *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/28/1994 BR : Created.                                              *
 *=========================================================================*/
bool Load_Game(int id)
{
    char name[_MAX_FNAME + _MAX_EXT];

    /*
    **	Generate the filename to load
    */
    sprintf(name, "SAVEGAME.%03d", id);

    return Load_Game(name);
}

/*
** Version that takes a file name instead. ST - 9/9/2019 11:13AM
*/
bool Load_Game(const char* file_name)
{
    SaveGame save;

    if (!SaveGame::From_File(file_name, save)) {
        return false;
    }

    const auto& scenario = save.Header.ScenarioID;
    const auto house = save.Header.Parse_Player_House_Type();

    /*
    **	Set the required CD to be in the drive according to the scenario
    **	loaded.
    */
    if (RequiredCD != -2) {
        if (scenario >= 20 && scenario < 60 && GameToPlay == GAME_NORMAL) {
            RequiredCD = 2;
        } else {
            if (scenario >= 60) {
                /*
                ** This is a gateway bonus scenario
                */
                RequiredCD = -1;
            } else {
                if (house == HOUSE_GOOD) {
                    RequiredCD = 0;
                } else {
                    RequiredCD = 1;
                }
            }
        }
    }
    if (!Force_CD_Available(RequiredCD)) {
        Prog_End("Load_Game - CD not found", true);
        if (!RunningAsDLL) {
            exit(EXIT_FAILURE);
        }
        return false;
    }

    if (!save.Write_Globals()) {
        CNC_LOG_ERROR("Failed to load JSON save into game state");
        return false;
    }

    Map.Init_IO();
    Map.Flag_To_Redraw(true);

    Fixup_Scenario();

    ScenarioInit = 0;

    /*
    ** Fixup remap tables. ST - 2/28/2020 1:50PM
    ** Only fixup remap of multiplayer houses. On non-remaster renderer, remapping
    ** Nod breaks Nod radar color because it gets remapped to its primary color,
    ** which is LTBLUE where it is supposed to be RED. Since only multiplayer colors
    ** can change colors, this fix only makes sense on multiplayer houses
    ** - mrparrot 07/12/2021
    */
    for (HousesType house = HOUSE_MULTI1; house < MPlayerCount + MPlayerGhosts; house++) {
        HouseClass* hptr = HouseClass::As_Pointer(house);
        if (hptr && hptr->IsActive) {
            hptr->Init_Data(hptr->RemapColor, hptr->ActLike, hptr->Credits);
        }
    }

#ifdef DEMO
    if (Scen.Scenario != 10 && Scen.Scenario != 1 && Scen.Scenario != 6) {
        Clear_Scenario();
        return (false);
    }
#endif

    Call_Back();
    return (true);
}

/***************************************************************************
 * Save_Misc_Values -- saves miscellaneous variables                       *
 *                                                                         *
 * INPUT:                                                                  *
 *      file      file to use for writing                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      true = success, false = failure                                    *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/29/1994 BR : Created.                                              *
 *=========================================================================*/
bool Save_Misc_Values(FileClass& file)
{
    int i, j;
    int count;        // # ptrs in 'CurrentObject'
    ObjectClass* ptr; // for saving 'CurrentObject' ptrs

    /*
    **	Player's House.
    */
    if (file.Write(&PlayerPtr, sizeof(PlayerPtr)) != sizeof(PlayerPtr)) {
        return (false);
    }

    /*
    **	Save this scenario number.
    */
    if (file.Write(&Scen.Scenario, sizeof(Scen.Scenario)) != sizeof(Scen.Scenario)) {
        return (false);
    }

    /*
    **	Save difficulty.
    */
    if (file.Write(&Scen.Difficulty, sizeof(Scen.Difficulty)) != sizeof(Scen.Difficulty)) {
        return (false);
    }

    /*
    **	Save AI difficulty.
    */
    if (file.Write(&Scen.CDifficulty, sizeof(Scen.CDifficulty)) != sizeof(Scen.CDifficulty)) {
        return (false);
    }
    /*
    **	Save frame #.
    */
    if (file.Write(&Frame, sizeof(Frame)) != sizeof(Frame)) {
        return (false);
    }

    /*
    **	Save VQ Movie names.
    */
    if (file.Write(WinMovie, sizeof(WinMovie)) != sizeof(WinMovie)) {
        return (false);
    }

    if (file.Write(LoseMovie, sizeof(LoseMovie)) != sizeof(LoseMovie)) {
        return (false);
    }

    /*
    **	Save currently-selected objects list.
    **	Save the # of ptrs in the list.
    */
    for (i = 0; i < SelectedObjectsType::COUNT; i++) {
        DynamicVectorClass<ObjectClass*>& selection = CurrentObject.Raw(i);
        count = selection.Count();
        if (file.Write(&count, sizeof(count)) != sizeof(count)) {
            return (false);
        }

        /*
        **	Save the pointers.
        */
        for (j = 0; j < count; j++) {
            ptr = selection[j];
            if (file.Write(&ptr, sizeof(ptr)) != sizeof(ptr)) {
                return (false);
            }
        }
    }

    /*
    **	Save the list of waypoints.
    */
    if (file.Write(Scen.Waypoint, sizeof(Scen.Waypoint)) != sizeof(Scen.Waypoint)) {
        return (false);
    }

    file.Write(&ScenDir, sizeof(ScenDir));
    file.Write(&ScenVar, sizeof(ScenVar));
    file.Write(&Scen.CarryOverMoney, sizeof(Scen.CarryOverMoney));
    file.Write(&Scen.CarryOverPercent, sizeof(Scen.CarryOverPercent));
    file.Write(&BuildLevel, sizeof(BuildLevel));
    file.Write(BriefMovie, sizeof(BriefMovie));
    file.Write(Scen.Views, sizeof(Scen.Views));
    file.Write(&EndCountDown, sizeof(EndCountDown));
    file.Write(Scen.BriefingText, sizeof(Scen.BriefingText));
    file.Write(Scen.FileName, sizeof(Scen.FileName));

    // This is new...
    file.Write(ActionMovie, sizeof(ActionMovie));
    file.Write(&TempleIoned, sizeof(TempleIoned));
    file.Write(&AreThingiesEnabled, sizeof(AreThingiesEnabled));

    return (true);
}

/***********************************************************************************************
 * Load_Misc_Values -- Loads miscellaneous variables.                                          *
 *                                                                                             *
 * INPUT:   file  -- The file to load the misc values from.                                    *
 *                                                                                             *
 * OUTPUT:  Was the misc load process successful?                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/24/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
bool Load_Misc_Values(FileClass& file)
{
    int i, j;
    int count;        // # ptrs in 'CurrentObject'
    ObjectClass* ptr; // for loading 'CurrentObject' ptrs

    /*
    **	Player's House.
    */
    if (file.Read(&PlayerPtr, sizeof(PlayerPtr)) != sizeof(PlayerPtr)) {
        return (false);
    }

    /*
    **	Read this scenario number.
    */
    if (file.Read(&Scen.Scenario, sizeof(Scen.Scenario)) != sizeof(Scen.Scenario)) {
        return (false);
    }

    /*
    **	Read difficulty.
    */
    if (file.Read(&Scen.Difficulty, sizeof(Scen.Difficulty)) != sizeof(Scen.Difficulty)) {
        return (false);
    }

    /*
    **	Read AI difficulty.
    */
    if (file.Read(&Scen.CDifficulty, sizeof(Scen.CDifficulty)) != sizeof(Scen.CDifficulty)) {
        return (false);
    }

    /*
    **	Load frame #.
    */
    if (file.Read(&Frame, sizeof(Frame)) != sizeof(Frame)) {
        return (false);
    }

    /*
    **	Load VQ Movie names.
    */
    if (file.Read(WinMovie, sizeof(WinMovie)) != sizeof(WinMovie)) {
        return (false);
    }

    if (file.Read(LoseMovie, sizeof(LoseMovie)) != sizeof(LoseMovie)) {
        return (false);
    }

    for (i = 0; i < SelectedObjectsType::COUNT; i++) {
        /*
        **	Load currently-selected objects list.
        **	Load the # of ptrs in the list.
        */
        DynamicVectorClass<ObjectClass*>& selection = CurrentObject.Raw(i);
        if (file.Read(&count, sizeof(count)) != sizeof(count)) {
            return (false);
        }

        /*
        **	Load the pointers.
        */
        for (j = 0; j < count; j++) {
            if (file.Read(&ptr, sizeof(ptr)) != sizeof(ptr)) {
                return (false);
            }
            selection.Add(ptr); // add to the list
        }
    }

    /*
    **	Save the list of waypoints.
    */
    if (file.Read(Scen.Waypoint, sizeof(Scen.Waypoint)) != sizeof(Scen.Waypoint)) {
        return (false);
    }

    file.Read(&ScenDir, sizeof(ScenDir));
    file.Read(&ScenVar, sizeof(ScenVar));
    file.Read(&Scen.CarryOverMoney, sizeof(Scen.CarryOverMoney));
    file.Read(&Scen.CarryOverPercent, sizeof(Scen.CarryOverPercent));
    file.Read(&BuildLevel, sizeof(BuildLevel));
    file.Read(BriefMovie, sizeof(BriefMovie));
    file.Read(Scen.Views, sizeof(Scen.Views));
    file.Read(&EndCountDown, sizeof(EndCountDown));
    file.Read(Scen.BriefingText, sizeof(Scen.BriefingText));

    if (file.Seek(0, SEEK_CUR) < file.Size()) {
        file.Read(Scen.FileName, sizeof(Scen.FileName));
    }

    if (file.Seek(0, SEEK_CUR) < file.Size()) {
        file.Read(ActionMovie, sizeof(ActionMovie));
    }

    if (file.Seek(0, SEEK_CUR) < file.Size()) {
        file.Read(&TempleIoned, sizeof(TempleIoned));
    }

    if (file.Seek(0, SEEK_CUR) < file.Size()) {
        file.Read(&AreThingiesEnabled, sizeof(AreThingiesEnabled));
    }

    return (true);
}

/*
** ST - 9/26/2019 11:43AM
*/
extern void DLL_Code_Pointers(void);
extern void DLL_Decode_Pointers(void);

/***********************************************************************************************
 * Code_All_Pointers -- Code all pointers.                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/24/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
void Code_All_Pointers(void)
{
    int i, j;

    /*
    **	The Map.
    */
    Map.Code_Pointers();

    /*
    **	The ArrayOf's.
    */
    TeamTypes.Code_Pointers();
    Teams.Code_Pointers();
    Triggers.Code_Pointers();
    Aircraft.Code_Pointers();
    Anims.Code_Pointers();
    Buildings.Code_Pointers();
    Bullets.Code_Pointers();
    Infantry.Code_Pointers();
    Overlays.Code_Pointers();
    Smudges.Code_Pointers();
    Templates.Code_Pointers();
    Terrains.Code_Pointers();
    Units.Code_Pointers();
    Factories.Code_Pointers();

    /*
    **	The Layers.
    */
    Logic.Code_Pointers();
    for (i = 0; i < LAYER_COUNT; i++) {
        Map.Layer[i].Code_Pointers();
    }

    /*
    **	The Score.
    */
    Score.Code_Pointers();

    /*
    **	The Base.
    */
    Base.Code_Pointers();

    /*
    **	PlayerPtr.
    */
    PlayerPtr = (HouseClass*)(PlayerPtr->Class->House);

    /*
    **	Currently-selected objects.
    */
    for (i = 0; i < SelectedObjectsType::COUNT; i++) {
        DynamicVectorClass<ObjectClass*>& selection = CurrentObject.Raw(i);
        for (j = 0; j < selection.Count(); j++) {
            selection[j] = (ObjectClass*)(intptr_t)selection[j]->As_Target();
        }
    }
#ifdef REMASTER_BUILD
    /*
    ** DLL data
    */
    DLL_Code_Pointers();
#endif
    /*
    ** Houses must be coded last, because the Class->House member of the HouseClass
    ** is used to code HouseClass pointers for all other objects, and if Class is
    ** coded, it will point to a meaningless value.
    */
    Houses.Code_Pointers();
}

/***********************************************************************************************
 * Decode_All_Pointers -- Decodes all pointers.                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/24/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
void Decode_All_Pointers(void)
{
    int i, j;

    /*
    **	The Map.
    */
    Map.Decode_Pointers();

    /*
    ** Decode houses first, so we can properly decode all other objects'
    ** House pointers
    */
    Houses.Decode_Pointers();
#ifdef REMASTER_BUILD
    /*
    ** DLL data
    */
    DLL_Decode_Pointers();
#endif
    /*
    **	The ArrayOf's.
    */
    TeamTypes.Decode_Pointers();
    Teams.Decode_Pointers();
    Triggers.Decode_Pointers();
    Aircraft.Decode_Pointers();
    Anims.Decode_Pointers();
    Buildings.Decode_Pointers();
    Bullets.Decode_Pointers();
    Infantry.Decode_Pointers();
    Overlays.Decode_Pointers();
    Smudges.Decode_Pointers();
    Templates.Decode_Pointers();
    Terrains.Decode_Pointers();
    Units.Decode_Pointers();
    Factories.Decode_Pointers();

    /*
    **	The Layers.
    */
    Logic.Decode_Pointers();
    for (i = 0; i < LAYER_COUNT; i++) {
        Map.Layer[i].Decode_Pointers();
    }

    /*
    **	The Score.
    */
    Score.Decode_Pointers();

    /*
    **	The Base.
    */
    Base.Decode_Pointers();

    /*
    **	PlayerPtr.
    */
    // PlayerPtr = HouseClass::As_Pointer((HousesType)(intptr_t)PlayerPtr);
    // Whom = PlayerPtr->Class->House;
    // switch (PlayerPtr->Class->House) {
    // case HOUSE_GOOD:
    //     ScenPlayer = SCEN_PLAYER_GDI;
    //     break;
    //
    // case HOUSE_BAD:
    //     ScenPlayer = SCEN_PLAYER_NOD;
    //     break;
    //
    // case HOUSE_JP:
    //     ScenPlayer = SCEN_PLAYER_JP;
    //     break;
    // }
    // Check_Ptr(PlayerPtr, __FILE__, __LINE__);
    //
    // if (PlayerPtr->ActLike == HOUSE_JP) {
    //     ScenPlayer = SCEN_PLAYER_JP;
    // }
    Set_Scenario_Name(Scen.ScenarioName, Scen.Scenario, ScenPlayer, ScenDir, ScenVar);

    /*
    **	Currently-selected objects.
    */
    for (i = 0; i < SelectedObjectsType::COUNT; i++) {
        DynamicVectorClass<ObjectClass*>& selection = CurrentObject.Raw(i);
        for (j = 0; j < selection.Count(); j++) {
            uintptr_t target_as_object_ptr = reinterpret_cast<uintptr_t>(selection[j]);
            TARGET target = (TARGET)target_as_object_ptr;
            selection[j] = As_Object(target);
            Check_Ptr(selection[j], __FILE__, __LINE__);
        }
    }

    /*
    **	Last-Minute Fixups; to resolve these pointers properly requires all other
    **	pointers to be loaded & decoded.
    */
    if (Map.PendingObjectPtr) {
        Map.PendingObject = &Map.PendingObjectPtr->Class_Of();
        Check_Ptr((void*)Map.PendingObject, __FILE__, __LINE__);
        Map.Set_Cursor_Shape(Map.PendingObject->Occupy_List(true));
    } else {
        Map.PendingObject = 0;
        Map.Set_Cursor_Shape(0);
    }
}

/***************************************************************************
 * Get_Savefile_Info -- gets description, scenario #, house                *
 *                                                                         *
 * INPUT:                                                                  *
 *      id         numerical ID, for the file extension                    *
 *      buf      buffer to store description in                            *
 *      scenp      ptr to variable to hold scenario                        *
 *      housep   ptr to variable to hold house                             *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      true = OK, false = error (save-game file invalid)                  *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/12/1995 BR : Created.                                              *
 *=========================================================================*/
bool Get_Savefile_Info(int id, char* buf, unsigned* scenp, HousesType* housep)
{
    CDFileClass file;
    char name[_MAX_FNAME + _MAX_EXT];
    unsigned int version;
    char descr_buf[DESCRIP_MAX];

    /*
    **	Generate the filename to load
    */
    sprintf(name, "SAVEGAME.%03d", id);

    /*
    **	If the file opens OK, read the file
    */
    if (file.Open(name, READ)) {

        /*
        **	Read in the description, scenario #, and the house
        */
        if (file.Read(descr_buf, DESCRIP_MAX) != DESCRIP_MAX) {
            file.Close();
            return (false);
        }

        descr_buf[strlen(descr_buf) - 2] = '\0'; // trim off CR/LF
        strcpy(buf, descr_buf);

        if (file.Read(scenp, sizeof(unsigned)) != sizeof(unsigned)) {
            file.Close();
            return (false);
        }

        if (file.Read(housep, sizeof(HousesType)) != sizeof(HousesType)) {
            file.Close();
            return (false);
        }

        /*
        **	Read & verify the save-game version #
        */
        if (file.Read(&version, sizeof(version)) != sizeof(version)) {
            file.Close();
            return (false);
        }

        if (version != SAVEGAME_VERSION) {
            file.Close();
            return (false);
        }

        file.Close();

        return (true);
    }
    return (false);
}

/***************************************************************************
 * TechnoType_To_Target -- converts TechnoTypeClass to TARGET              *
 *                                                                         *
 * INPUT:                                                                  *
 *      ptr      pointer to convert                                        *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      target value                                                       *
 *                                                                         *
 * WARNINGS:                                                               *
 *      Be certain that you only use the returned target value by passing  *
 *      it to Target_To_TechnoType; do NOT call As_Techno, or you'll get   *
 *      a totally invalid pointer.                                         *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/12/1995 BR : Created.                                              *
 *=========================================================================*/
TARGET TechnoType_To_Target(TechnoTypeClass const* ptr)
{
    TARGET target;

    switch (ptr->What_Am_I()) {
    case RTTI_INFANTRYTYPE:
        target = Build_Target(KIND_INFANTRY, ((InfantryTypeClass const*)ptr)->Type);
        break;

    case RTTI_UNITTYPE:
        target = Build_Target(KIND_UNIT, ((UnitTypeClass const*)ptr)->Type);
        break;

    case RTTI_AIRCRAFTTYPE:
        target = Build_Target(KIND_AIRCRAFT, ((AircraftTypeClass const*)ptr)->Type);
        break;

    case RTTI_BUILDINGTYPE:
        target = Build_Target(KIND_BUILDING, ((BuildingTypeClass const*)ptr)->Type);
        break;

    default:
        target = 0;
        break;
    }

    return (target);
}

/***************************************************************************
 * Target_To_TechnoType -- converts TARGET to TechnoTypeClass              *
 *                                                                         *
 * INPUT:                                                                  *
 *      target      TARGET value to convert                                *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      pointer to the TechnoTypeClass for this target value               *
 *                                                                         *
 * WARNINGS:                                                               *
 *      The TARGET value MUST have been generated with TechnoType_To_Target;*
 *      If you give this routine a target generated by an As_Target()      *
 *      routine, it will return a bogus pointer.                           *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/12/1995 BR : Created.                                              *
 *=========================================================================*/
TechnoTypeClass const* Target_To_TechnoType(TARGET target)
{
    switch (Target_Kind(target)) {
    case KIND_INFANTRY:
        return (&InfantryTypeClass::As_Reference((InfantryType)Target_Value(target)));

    case KIND_UNIT:
        return (&UnitTypeClass::As_Reference((UnitType)Target_Value(target)));

    case KIND_AIRCRAFT:
        return (&AircraftTypeClass::As_Reference((AircraftType)Target_Value(target)));

    case KIND_BUILDING:
        return (&BuildingTypeClass::As_Reference((StructType)Target_Value(target)));
    }
    return (NULL);
}

#if 0
/****************************************************************************
Dump routine: prints everything about everything related to the Save/Load
process (OK, not exactly everything, but lots of stuff)
****************************************************************************/
void Dump(void)
{
	int i,j;
	FILE *fp;
	char *layername[] = {
		"Ground",
		"Air",
		"Top"
	};

	/*
	------------------------------- Open file --------------------------------
	*/
	fp = fopen("dump.txt","wt");

	/*
	------------------------------ Logic Layer -------------------------------
	*/
	fprintf(fp,"--------------------- Logic Layer ---------------------\n");
	fprintf(fp,"Count: %d\n",Logic.Count());
	for (j = 0; j < Logic.Count(); j++) {
		fprintf(fp, "Entry %d: %x \n",j,Logic[j]);
	}
	fprintf(fp,"\n");

	/*
	------------------------------- Map Layers -------------------------------
	*/
	for (i = 0; i < LAYER_COUNT; i++) {
		fprintf(fp,"----------------- Map Layer %s ---------------------\n",
			layername[i]);
		fprintf(fp,"Count: %d\n",Map.Layer[i].Count());
		for (j = 0; j < Map.Layer[i].Count(); j++) {
			fprintf(fp, "Entry %d: %x \n",j,Map.Layer[i][j]);
		}
	}
	fprintf(fp,"\n");

	fprintf(fp,"------------------ TeamTypes --------------------------\n");
	fprintf(fp,"ActiveCount: %d\n",TeamTypes.ActiveCount);
	for (i = 0; i < TEAMTYPE_MAX; i++) {
		fprintf(fp,"Entry %d: Active:%d Name:%s\n",i,TeamTypes[i].IsActive,
			TeamTypes[i].Get_Name());
	}
	fprintf(fp,"\n");

	fprintf(fp,"------------------ Teams --------------------------\n");
	fprintf(fp,"ActiveCount: %d\n",Teams.ActiveCount);
	for (i = 0; i < TEAM_MAX; i++) {
		fprintf(fp,"Entry %d: Active:%d Name:%s\n",i,Teams[i].IsActive,
			Teams[i].Class->Get_Name());
	}
	fprintf(fp,"\n");

	fprintf(fp,"------------------ Triggers --------------------------\n");
	fprintf(fp,"ActiveCount: %d\n",Triggers.ActiveCount);
	for (i = 0; i < TRIGGER_MAX; i++) {
		fprintf(fp,"Entry %d: Active:%d Name:%s\n",i,Triggers[i].IsActive,
			Triggers[i].Get_Name());
	}
	fprintf(fp,"\n");

	fprintf(fp,"------------------ Aircraft --------------------------\n");
	fprintf(fp,"ActiveCount: %d\n",Aircraft.ActiveCount);
	for (i = 0; i < AIRCRAFT_MAX; i++) {
		fprintf(fp,"Entry %d: Active:%d \n",i,Aircraft[i].IsActive);
	}
	fprintf(fp,"\n");

	fprintf(fp,"------------------ Anims --------------------------\n");
	fprintf(fp,"ActiveCount: %d\n",Anims.ActiveCount);
	for (i = 0; i < ANIM_MAX; i++) {
		fprintf(fp,"Entry %d: Active:%d \n",i,Anims[i].IsActive);
	}
	fprintf(fp,"\n");

	fprintf(fp,"------------------ Buildings --------------------------\n");
	fprintf(fp,"ActiveCount: %d\n",Buildings.ActiveCount);
	for (i = 0; i < BUILDING_MAX; i++) {
		fprintf(fp,"Entry %d: Active:%d \n",i,Buildings[i].IsActive);
	}
	fprintf(fp,"\n");

	fprintf(fp,"------------------ Bullets --------------------------\n");
	fprintf(fp,"ActiveCount: %d\n",Bullets.ActiveCount);
	for (i = 0; i < BULLET_MAX; i++) {
		fprintf(fp,"Entry %d: Active:%d \n",i,Bullets[i].IsActive);
	}
	fprintf(fp,"\n");

	fprintf(fp,"------------------ Infantry --------------------------\n");
	fprintf(fp,"ActiveCount: %d\n",Infantry.ActiveCount);
	for (i = 0; i < INFANTRY_MAX; i++) {
		fprintf(fp,"Entry %d: Active:%d \n",i,Infantry[i].IsActive);
	}
	fprintf(fp,"\n");

	fprintf(fp,"------------------ Overlays --------------------------\n");
	fprintf(fp,"ActiveCount: %d\n",Overlays.ActiveCount);
	for (i = 0; i < OVERLAY_MAX; i++) {
		fprintf(fp,"Entry %d: Active:%d \n",i,Overlays[i].IsActive);
	}
	fprintf(fp,"\n");

	fprintf(fp,"------------------ Reinforcements --------------------------\n");
	fprintf(fp,"ActiveCount: %d\n",Reinforcements.ActiveCount);
	for (i = 0; i < REINFORCEMENT_MAX; i++) {
		fprintf(fp,"Entry %d: Active:%d \n",i,Reinforcements[i].IsActive);
	}
	fprintf(fp,"\n");

	fprintf(fp,"------------------ Smudges --------------------------\n");
	fprintf(fp,"ActiveCount: %d\n",Smudges.ActiveCount);
	for (i = 0; i < SMUDGE_MAX; i++) {
		fprintf(fp,"Entry %d: Active:%d \n",i,Smudges[i].IsActive);
	}
	fprintf(fp,"\n");

	fprintf(fp,"------------------ Templates --------------------------\n");
	fprintf(fp,"ActiveCount: %d\n",Templates.ActiveCount);
	for (i = 0; i < TEMPLATE_MAX; i++) {
		fprintf(fp,"Entry %d: Active:%d \n",i,Templates[i].IsActive);
	}
	fprintf(fp,"\n");

	fprintf(fp,"------------------ Terrains --------------------------\n");
	fprintf(fp,"ActiveCount: %d\n",Terrains.ActiveCount);
	for (i = 0; i < TERRAIN_MAX; i++) {
		fprintf(fp,"Entry %d: Active:%d \n",i,Terrains[i].IsActive);
	}
	fprintf(fp,"\n");

	fprintf(fp,"------------------ Units --------------------------\n");
	fprintf(fp,"ActiveCount: %d\n",Units.ActiveCount);
	for (i = 0; i < UNIT_MAX; i++) {
		fprintf(fp,"Entry %d: Active:%d \n",i,Units[i].IsActive);
	}
	fprintf(fp,"\n");

	fprintf(fp,"------------------ Factories --------------------------\n");
	fprintf(fp,"ActiveCount: %d\n",Factories.ActiveCount);
	for (i = 0; i < FACTORY_MAX; i++) {
		fprintf(fp,"Entry %d: Active:%d \n",i,Factories[i].IsActive);
	}
	fprintf(fp,"\n");

	fclose(fp);

	/*
	---------------------------- Flush the cache -----------------------------
	*/
	fp = fopen("dummy.bin","wt");
	for (i = 0; i < 100; i++) {
		fprintf(fp,"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
	}
	fclose(fp);
}
#endif