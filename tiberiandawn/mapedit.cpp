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

/* $Header:   F:\projects\c&c\vcs\code\mapedit.cpv   2.18   16 Oct 1995 16:48:40   JOE_BOSTIC  $ */
/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S        **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : MAPEDIT.CPP                              *
 *                                                                         *
 *                   Programmer : Bill Randolph                            *
 *                                                                         *
 *                   Start Date : October 20, 1994                         *
 *                                                                         *
 *                  Last Update : February 2, 1995   [BR]                  *
 *                                                                         *
 *-------------------------------------------------------------------------*
 *   Map Editor overloaded routines & utility routines                     *
 *-------------------------------------------------------------------------*
 * Map Editor modules:                                                     *
 * (Yes, they're all one huge class.)                                      *
 *      mapedit.cpp:   overloaded routines, utility routines               *
 *      mapeddlg.cpp:   map editor dialogs, most of the main menu options  *
 *      mapedplc.cpp:   object-placing routines                            *
 *      mapedsel.cpp:   object-selection & manipulation routines           *
 *      mapedtm.cpp:   team-editing routines                               *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   MapEditClass::MapEditClass -- class constructor                       *
 *   MapEditClass::One_Time -- one-time initialization                     *
 *   MapEditClass::Read_INI -- overloaded Read_INI function                *
 *   MapEditClass::Clear_List -- clears the internal choosable object list *
 *   MapEditClass::Add_To_List -- adds a TypeClass to the choosable list   *
 *   MapEditClass::AI -- The map editor's main logic                       *
 *   MapEditClass::Draw_It -- overloaded Redraw routine                    *
 *   MapEditClass::Main_Menu -- main menu processor for map editor         *
 *   MapEditClass::AI_Menu -- menu of AI options                           *
 *   MapEditClass::Mouse_Moved -- checks for mouse motion                  *
 *   MapEditClass::Verify_House -- sees if given house can own given obj   *
 *   MapEditClass::Cycle_House -- finds next valid house for object type   *
 *   MapEditClass::Trigger_Needs_Team -- tells if a trigger needs a team   *
 *   MapEditClass::Fatal -- exits with error message                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "function.h"
#include "common/framelimit.h"
#include "lua/scenariolua.h"
#include "ccini.h"
#include "tiberiandawnsettings.h"

#ifdef SCENARIO_EDITOR

/*
****************************** Globals/Externs ******************************
*/
/*...........................................................................
Array of all missions supported by the map editor
...........................................................................*/
MissionType MapEditClass::MapEditMissions[NUM_EDIT_MISSIONS] = {
    MISSION_GUARD,
    MISSION_STICKY,
    MISSION_HARVEST,
    MISSION_GUARD_AREA,
    MISSION_RETURN,
    MISSION_AMBUSH,
    MISSION_HUNT,
    MISSION_SLEEP,
};

/*...........................................................................
For menu processing
...........................................................................*/
extern int UnknownKey; // in menus.cpp

char MapEditClass::HealthBuf[20];
char MapEditClass::BaseText[6] = "Base:";

/***************************************************************************
 * MapEditClass::MapEditClass -- class constructor                         *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/20/1994 BR : Created.                                              *
 *=========================================================================*/
MapEditClass::MapEditClass(void)
{
    /*
    **	Init data members.
    */
    ScenVar = SCEN_VAR_A;
    ObjCount = 0;
    LastChoice = 0;
    LastHouse = HOUSE_GOOD;
    GrabbedObject = nullptr;
    for (int i = 0; i < NUM_EDIT_CLASSES; i++) {
        NumType[i] = 0;
        TypeOffset[i] = 0;
    }
    Scen.Waypoint[WAYPT_HOME] = 0;
    CurrentCell = 0;
    CurTrigger = nullptr;
    CurWaypoint = WAYPT_COUNT;
    GrabbedOverlay = false;
    GrabbedOverlayOrigin = 0;
    Changed = 0;
    LMouseDown = 0;
    BaseBuilding = 0;
    BasePercent = 100;
}

/***************************************************************************
 * MapEditClass::One_Time -- one-time initialization                       *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   02/02/1995 BR : Created.                                              *
 *=========================================================================*/
void MapEditClass::One_Time(void)
{
    MouseClass::One_Time();

    HeaderX = 0;
    HeaderY = 0;
    HeaderW = SeenBuff.Get_Width();
    HeaderH = Get_Tab_Height();

    PopupDialogVisible = false;

    FooterW = HeaderW;
    FooterH = HeaderH;
    FooterX = 0;
    FooterY = SeenBuff.Get_Height() - FooterH;

    EditorSidebar.W = SeenBuff.Get_Width() / 4;
    EditorSidebar.H = SeenBuff.Get_Height() - HeaderH - FooterH - 3;
    EditorSidebar.X = SeenBuff.Get_Width() - EditorSidebar.W;
    EditorSidebar.Y = HeaderY + HeaderH + 2;

    /*------------------------------------------------------------------------
    Create the pop-up controls
    ------------------------------------------------------------------------*/
    /*........................................................................
    The map: a single large "button"
    ........................................................................*/
    // MapArea = new ControlClass(MAP_AREA,0,8,312,192, GadgetClass::LEFTPRESS |
    // GadgetClass::LEFTRELEASE, false);
    MapArea = new ControlClass(
        MAP_AREA,
        0,
        HeaderX + HeaderH + 2,
        EditorSidebar.X - 1,
        (FooterY - 1) - (HeaderH + 2),
        GadgetClass::LEFTPRESS | GadgetClass::LEFTRELEASE,
        false
    );

    // make all control positions relative to POPUP_GDI and use offset center X co-ord so the UI layout is centered
    const auto POPUP_GDI_X = (MapArea->Width / 2)
        - ((POPUP_GDI_W + POPUP_FACEBOX_W + POPUP_HEALTH_W + POPUP_MISSION_W + (CONTROL_MARGIN * 3)) / 2);
    const auto POPUP_GDI_Y = SeenBuff.Get_Height() - (FooterH + (CONTROL_MARGIN * 4) + POPUP_FACEBOX_H);

    const auto POPUP_NOD_X = POPUP_GDI_X;
    const auto POPUP_NOD_Y = POPUP_GDI_Y + POPUP_GDI_H;

    const auto POPUP_NEUTRAL_X = POPUP_GDI_X;
    const auto POPUP_NEUTRAL_Y = POPUP_NOD_Y + POPUP_NOD_H;

    const auto POPUP_MULTI1_X = POPUP_GDI_X;
    const auto POPUP_MULTI1_Y = POPUP_GDI_Y;

    const auto POPUP_MULTI2_X = POPUP_GDI_X + (CONTROL_MARGIN * 10);
    const auto POPUP_MULTI2_Y = POPUP_MULTI1_Y;

    const auto POPUP_MULTI3_X = POPUP_MULTI1_X;
    const auto POPUP_MULTI3_Y = POPUP_NOD_Y;

    const auto POPUP_MULTI4_X = POPUP_MULTI2_X;
    const auto POPUP_MULTI4_Y = POPUP_MULTI3_Y;

    const auto POPUP_FACEBOX_X = POPUP_GDI_X + POPUP_GDI_W + CONTROL_MARGIN;
    const auto POPUP_FACEBOX_Y = POPUP_GDI_Y - CONTROL_MARGIN;

    const auto POPUP_HEALTH_X = POPUP_FACEBOX_X + POPUP_FACEBOX_W + CONTROL_MARGIN;
    const auto POPUP_HEALTH_Y = POPUP_GDI_Y + (CONTROL_MARGIN * 4);

    const auto POPUP_BASESTRUCTURE_X = POPUP_HEALTH_X + POPUP_HEALTH_W + CONTROL_MARGIN;
    const auto POPUP_BASESTRUCTURE_Y = POPUP_HEALTH_Y - (POPUP_HEALTH_H / 2);

    const auto POPUP_BASEPRIORITY_X = POPUP_BASESTRUCTURE_X;
    const auto POPUP_BASEPRIORITY_Y = POPUP_BASESTRUCTURE_Y + POPUP_BASESTRUCTURE_SIZE + (CONTROL_MARGIN * 2);

    const auto POPUP_MISSION_X = POPUP_HEALTH_X + POPUP_HEALTH_W + CONTROL_MARGIN;
    const auto POPUP_MISSION_Y = POPUP_FACEBOX_Y;

    EditorSidebar.Init(this);

    /*........................................................................
    House buttons
    ........................................................................*/
    GDIButton = new TextButtonClass(POPUP_GDI,
                                    "GDI",
                                    TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                                    POPUP_GDI_X,
                                    POPUP_GDI_Y,
                                    POPUP_GDI_W,
                                    POPUP_GDI_H);

    NODButton = new TextButtonClass(POPUP_NOD,
                                    "NOD",
                                    TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                                    POPUP_NOD_X,
                                    POPUP_NOD_Y,
                                    POPUP_NOD_W,
                                    POPUP_NOD_H);

    NeutralButton = new TextButtonClass(POPUP_NEUTRAL,
                                        "Neutral",
                                        TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                                        POPUP_NEUTRAL_X,
                                        POPUP_NEUTRAL_Y,
                                        POPUP_NEUTRAL_W,
                                        POPUP_NEUTRAL_H);

    Multi1Button = new TextButtonClass(POPUP_MULTI1,
                                       "M1",
                                       TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                                       POPUP_MULTI1_X,
                                       POPUP_MULTI1_Y,
                                       POPUP_MULTI1_W,
                                       POPUP_MULTI1_H);

    Multi2Button = new TextButtonClass(POPUP_MULTI2,
                                       "M2",
                                       TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                                       POPUP_MULTI2_X,
                                       POPUP_MULTI2_Y,
                                       POPUP_MULTI2_W,
                                       POPUP_MULTI2_H);

    Multi3Button = new TextButtonClass(POPUP_MULTI3,
                                       "M3",
                                       TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                                       POPUP_MULTI3_X,
                                       POPUP_MULTI3_Y,
                                       POPUP_MULTI3_W,
                                       POPUP_MULTI3_H);

    Multi4Button = new TextButtonClass(POPUP_MULTI4,
                                       "M4",
                                       TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                                       POPUP_MULTI4_X,
                                       POPUP_MULTI4_Y,
                                       POPUP_MULTI4_W,
                                       POPUP_MULTI4_H);

    /*........................................................................
    The mission list box
    ........................................................................*/
    MissionList = new ListClass(POPUP_MISSIONLIST,
                                POPUP_MISSION_X,
                                POPUP_MISSION_Y,
                                POPUP_MISSION_W,
                                POPUP_MISSION_H,
                                TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                                Hires_Retrieve("BTN-UP.SHP"),
                                Hires_Retrieve("BTN-DN.SHP"));

    for (int i = 0; i < NUM_EDIT_MISSIONS; i++) {
        MissionList->Add_Item(MissionClass::Mission_Name(MapEditMissions[i]));
    }

    /*........................................................................
    The health bar
    ........................................................................*/
    HealthGauge =
        new TriColorGaugeClass(POPUP_HEALTHGAUGE, POPUP_HEALTH_X, POPUP_HEALTH_Y, POPUP_HEALTH_W, POPUP_HEALTH_H);
    HealthGauge->Use_Thumb(true);
    HealthGauge->Set_Maximum(0x100);
    HealthGauge->Set_Red_Limit(0x3f - 1);
    HealthGauge->Set_Yellow_Limit(0x7f - 1);

    /*........................................................................
    The health text label
    ........................................................................*/
    HealthBuf[0] = 0;
    HealthText = new TextLabelClass(HealthBuf,
                                    POPUP_HEALTH_X + POPUP_HEALTH_W / 2,
                                    POPUP_HEALTH_Y + POPUP_HEALTH_H + 1,
                                    CC_GREEN,
                                    TPF_CENTER | TPF_FULLSHADOW | TPF_6PT_GRAD | TPF_USE_GRAD_PAL);

    /*........................................................................
    The facing dial
    ........................................................................*/
    FacingDial = new Dial8Class(
        POPUP_FACINGDIAL, POPUP_FACEBOX_X, POPUP_FACEBOX_Y, POPUP_FACEBOX_W, POPUP_FACEBOX_H, (DirType)0);

    /*........................................................................
    AI Base flag for structures
    ........................................................................*/
    static char base_structure_text[5] = "Base";

    IsBaseStructureCheckbox = new CheckBoxClass(POPUP_BASESTRUCTURE, POPUP_BASESTRUCTURE_X, POPUP_BASESTRUCTURE_Y, POPUP_BASESTRUCTURE_SIZE);
    IsBaseStructureText = new TextLabelClass(base_structure_text,
                                    IsBaseStructureCheckbox->X + (POPUP_BASESTRUCTURE_SIZE * 2) + 5,
                                    IsBaseStructureCheckbox->Y + ((POPUP_BASESTRUCTURE_SIZE - 6) / 6),
                                    CC_GREEN,
                                    TPF_CENTER | TPF_FULLSHADOW | TPF_6PT_GRAD | TPF_USE_GRAD_PAL);

    /*........................................................................
    AI Base ID textbox for structures
    ........................................................................*/
    static char base_id_text[8] = "Base ID";

    BaseStructureIdTextBox = new EditClass(
        POPUP_BASEID,
        BaseStructureIdBuffer,
        std::size(BaseStructureIdBuffer),
        TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
        POPUP_BASEPRIORITY_X,
        POPUP_BASEPRIORITY_Y,
        POPUP_BASESTRUCTURE_SIZE * 2,
        18,
        EditClass::NUMERIC
    );
    BaseStructureIdText = new TextLabelClass(
        base_id_text,
        POPUP_BASEPRIORITY_X + (POPUP_BASESTRUCTURE_SIZE * 2) + 2,
        POPUP_BASEPRIORITY_Y + 2,
        CC_GREEN,
        TPF_FULLSHADOW | TPF_6PT_GRAD | TPF_USE_GRAD_PAL
    );
    BaseStructureIdContext = nullptr;

    /*........................................................................
    Calculate popup dialog dimensions
    ........................................................................*/
    constexpr auto dialog_margin = CONTROL_MARGIN * 4;

    PopupDialogX = POPUP_GDI_X - dialog_margin;
    PopupDialogY = POPUP_GDI_Y - (dialog_margin * 2); // make room for caption text and livery

    // use bottom right controls as end points
    const auto end_x = POPUP_MISSION_X + POPUP_MISSION_W + dialog_margin;
    const auto end_y = POPUP_MISSION_Y + POPUP_MISSION_H + dialog_margin;

    PopupDialogW = end_x - PopupDialogX;
    PopupDialogH = end_y - PopupDialogY;

    /*........................................................................
    Calculate editor sidebar dimensions
    ........................................................................*/
    const auto sidebar_width_limit = SeenBuff.Get_Width() - (PopupDialogX + PopupDialogW + dialog_margin);

    /*........................................................................
    The base percent-built slider & its label
    ........................................................................*/
    const auto POPUP_BASE_X = (HeaderX + HeaderW) - POPUP_BASE_W - (CONTROL_MARGIN * 10);

    BaseGauge = new GaugeClass(POPUP_BASEPERCENT, POPUP_BASE_X, HeaderY, POPUP_BASE_W, HeaderH);
    BaseLabel = new TextLabelClass(
        BaseText, POPUP_BASE_X - 3, 0, CC_GREEN, TPF_RIGHT | TPF_NOSHADOW | TPF_6PT_GRAD | TPF_USE_GRAD_PAL);
    BaseGauge->Set_Maximum(100);
    BaseGauge->Set_Value(BasePercent);
}

/***********************************************************************************************
 * MapeditClass::Init_IO -- Reinitializes the radar map at scenario start.                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/22/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void MapEditClass::Init_IO(void)
{
    /*------------------------------------------------------------------------
    For normal game mode, jump to the parent's Init routine.
    ------------------------------------------------------------------------*/
    if (!Debug_Map) {

        MouseClass::Init_IO();

    } else {

        /*------------------------------------------------------------------------
        For editor mode, add the map area to the button input list
        ------------------------------------------------------------------------*/
        Buttons = nullptr;

        EditorSidebar.Add_This();
        Add_A_Button(*BaseGauge);
        Add_A_Button(*BaseLabel);
        Add_A_Button(*MapArea);
    }
}

/***************************************************************************
 * MapEditClass::Read_INI -- overloaded Read_INI function                  *
 *                                                                         *
 * Overloading this function gives the map editor a chance to initialize   *
 * certain values every time a new INI is read.                            *
 *                                                                         *
 * INPUT:                                                                  *
 *      buffer      INI staging area                                       *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/16/1994 BR : Created.                                              *
 *=========================================================================*/
void MapEditClass::Read_INI(CCINIClass& ini)
{
    /*
    ------------------------ Invoke parent's Read_INI ------------------------
    */
    MouseClass::Read_INI(ini);

    BasePercent = ini.Get_Int("Basic", "Percent", 0);
    BaseGauge->Set_Value(BasePercent);

    CurrentCell = Coord_Cell(Pixel_To_Coord(TacPixelX, TacPixelY));

    Changed = 0;

    /*------------------------------------------------------------------------
    Initialize addable objects list, required due to possible theater change
    ------------------------------------------------------------------------*/
    Clear_List();
    TemplateTypeClass::Prep_For_Add();
    OverlayTypeClass::Prep_For_Add();
    SmudgeTypeClass::Prep_For_Add();
    TerrainTypeClass::Prep_For_Add();
    UnitTypeClass::Prep_For_Add();
    InfantryTypeClass::Prep_For_Add();
    AircraftTypeClass::Prep_For_Add();
    BuildingTypeClass::Prep_For_Add();

    /*........................................................................
    Compute offset of each class type in the Objects array
    ........................................................................*/
    for (auto i = 0; i < NUM_EDIT_CLASSES; i++) {
        TypeOffset[i] = i == 0 ? 0 : TypeOffset[i - 1] + NumType[i - 1];
    }
}

/***************************************************************************
 * MapEditClass::Write_INI -- overloaded Read_INI function                 *
 *                                                                         *
 * INPUT:                                                                  *
 *      buffer      INI staging area                                       *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/16/1994 BR : Created.                                              *
 *=========================================================================*/
void MapEditClass::Write_INI(CCINIClass& ini)
{
    /*
    ----------------------- Invoke parent's Write_INI ------------------------
    */
    MouseClass::Write_INI(ini);

    /*
    ** Save the base's percent-built value; this must be saved into the BASIC
    ** section of the INI, since the Base section will be entirely erased
    ** by the Base's Write_INI routine.
    */
    ini.Put_Int("Basic", "Percent", BasePercent);

    if (Scen.ScenarioBasicName.has_value()) {
        ini.Put_String("Basic", "Name", *Scen.ScenarioBasicName);
    }

    ScenarioLua::Write_Lua_Script_Path(ini, Scen.LuaScriptPath);
}

/***************************************************************************
 * MapEditClass::Clear_List -- clears the internal choosable object list   *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/20/1994 BR : Created.                                              *
 *=========================================================================*/
void MapEditClass::Clear_List(void)
{
    /*------------------------------------------------------------------------
    Set # object type ptrs to 0, set NumType for each type to 0
    ------------------------------------------------------------------------*/
    ObjCount = 0;
    for (int i = 0; i < NUM_EDIT_CLASSES; i++) {
        NumType[i] = 0;
    }
}

/***************************************************************************
 * MapEditClass::Add_To_List -- adds a TypeClass to the choosable list     *
 *                                                                         *
 * Use this routine to add an object to the game object selection list.    *
 * This list is used by the Add_Object function. All items located in the  *
 * list will appear and be chooseable by that function. Make sure to       *
 * clear the list before adding a sequence of items to it. Clearing        *
 * the list is accomplished by the Clear_List() function.                  *
 *                                                                         *
 * INPUT:                                                                  *
 *      object      ptr to ObjectTypeClass to add                          *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      bool: was the object added to the list?  A failure could occur if  *
 *      NULL were passed in or the list is full.                           *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   06/04/1994 JLB : Created.                                             *
 *=========================================================================*/
bool MapEditClass::Add_To_List(ObjectTypeClass const* object)
{
    /*
    **	Add the object if there's room.
    */
    if (object && ObjCount < MAX_EDIT_OBJECTS) {
        Objects[ObjCount++] = object;

        /*
        **	Update type counters.
        */
        switch (object->What_Am_I()) {
        case RTTI_TEMPLATETYPE:
            NumType[0]++;
            break;

        case RTTI_OVERLAYTYPE:
            NumType[1]++;
            break;

        case RTTI_SMUDGETYPE:
            NumType[2]++;
            break;

        case RTTI_TERRAINTYPE:
            NumType[3]++;
            break;

        case RTTI_UNITTYPE:
            NumType[4]++;
            break;

        case RTTI_INFANTRYTYPE:
            NumType[5]++;
            break;

        case RTTI_AIRCRAFTTYPE:
            NumType[6]++;
            break;

        case RTTI_BUILDINGTYPE:
            NumType[7]++;
            break;
        }
        return (true);
    }

    return (false);
}

void MapEditClass::Exit_Editor() const
{
    if (!Debug_Map) {
        return;
    }

    Theme.Queue_Song(THEME_NONE);
    Stop_Speaking();
    Speak(VOX_CONTROL_EXIT);
    while (Is_Speaking()) {
        Call_Back();
    }
    GameActive = false;
    Debug_Map = false;
}

static bool Change_Base_Node_Id(char* id_str, const size_t str_len, EditClass& control, BuildingClass* building)
{
    if (building == nullptr) {
        return false;
    }

    if (!Base.Is_Node(building)) {
        return false;
    }

    try {
        const auto desired_node_idx = std::stoi(id_str);
        const auto node_to_move_ptr = Base.Get_Node(building);
        const auto current_node_idx = Base.Nodes.ID(node_to_move_ptr);

        auto result = Base.Nodes.Move(current_node_idx, desired_node_idx);

        if (!result) {
            // invalid id, reset to current node id
            sprintf(id_str, "%d", current_node_idx);
            control.Set_Text(id_str, str_len);
        }

        return result;
    } catch (const std::invalid_argument& _) {
    } catch (const std::out_of_range& _) {
    }

    control.Set_Color(RED);
    return false;
}

/***************************************************************************
 * MapEditClass::AI -- The map editor's main logic                         *
 *                                                                         *
 * This routine overloads the parent's (DisplayClass) AI function.         *
 * It checks for any input specific to map editing, and calls the parent   *
 * AI routine to handle scrolling and other mainstream map stuff.          *
 *                                                                         *
 * If this detects one of its special input keys, it sets 'input' to 0     *
 * before calling the parent AI routine; this prevents input conflict.     *
 *                                                                         *
 * SUPPORTED INPUT:                                                        *
 * General:                                                                *
 *      F2/RMOUSE:            main menu                                    *
 *      F6:                  toggles show-passable mode                    *
 *      HOME:                  go to the Home Cell (scenario's start position)*
 *      SHIFT-HOME:            set the Home Cell to the current TacticalCell*
 *      ESC:                  exits to DOS                                 *
 * Object Placement:                                                       *
 *      INSERT:               go into placement mode                       *
 *      ESC:                  exit placement mode                          *
 *      LEFT/RIGHT:          prev/next placement object                    *
 *      PGUP/PGDN:            prev/next placement category                 *
 *      HOME:                  1st placement object (clear template)       *
 *      h/H:                  toggle house of placement object             *
 *      LMOUSE:               place the placement object                   *
 *      MOUSE MOTION:         "paint" with the placement object            *
 * Object selection:                                                       *
 *      LMOUSE:               select & "grab" current object               *
 *                           If no object is present where the mouse is    *
 *                           clicked, the current object is de-selected    *
 *                           If the same object is clicked on, it stays    *
 *                           selected. Also displays the object-editing    *
 *                           gadgets.                                      *
 *      LMOUSE RLSE:         release currently-grabbed object              *
 *      MOUSE MOTION:         if an object is grabbed, moves the object    *
 *      SHIFT|ALT|ARROW:      moves object in that direction               *
 *      DELETE               deletes currently-selected object             *
 * Object-editing controls:                                                *
 *      POPUP_GDI:            makes GDI the owner of this object           *
 *      POPUP_NOD:            makes NOD the owner of this object           *
 *      POPUP_MISSIONLIST:   sets that mission for this object             *
 *      POPUP_HEALTHGAUGE:   sets that health value for this object        *
 *      POPUP_FACINGDIAL:      sets the object's facing                    *
 *                                                                         *
 * Changed is set when you:                                                *
 *      - place an object                                                  *
 *      - move a grabbed object                                            *
 *      - delete an object                                                 *
 *      - size the map                                                     *
 *      - create a new scenario                                            *
 *   Changed is cleared when you:                                          *
 *      - Save the scenario                                                *
 *      - Load a scenario                                                  *
 *      - Play the scenario                                                *
 *                                                                         *
 * INPUT:                                                                  *
 *      input      KN_ value, 0 if none                                    *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/20/1994 BR : Created.                                              *
 *=========================================================================*/
void MapEditClass::AI(KeyNumType& input, int x, int y)
{
    int rc;
    MissionType mission;
    int strength;
    CELL cell;
    int i;
    int found;           // for removing a waypoint label
    int waypt_idx;       // for labelling a waypoint
    BaseNodeClass* node; // for removing from an AI Base
    HousesType house;

    /*------------------------------------------------------------------------
    Trap 'F2' regardless of whether we're in game or editor mode
    ------------------------------------------------------------------------*/
    if (Debug_Flag) {
        if (/*(input == KN_F2 && Session == GAME_SOLO) ||*/ input == (KN_F2 | KN_CTRL_BIT)) {
            ScenarioInit = 0;

            /*
            ** If we're in editor mode & Changed is set, prompt for saving changes
            */
            if (Debug_Map && Changed) {
                rc = WWMessageBox().Process("Save Changes?", TXT_YES, TXT_NO);
                HiddenPage.Clear();
                Flag_To_Redraw(true);
                Render();
                /*
                ........................ User wants to save ........................
                */
                if (rc == 0) {

                    /*
                    ................ If save cancelled, abort game ..................
                    */
                    if (Save_Scenario() != 0) {
                        input = KN_NONE;
                    } else {
                        Changed = 0;
                        Go_Editor(!Debug_Map);
                    }
                } else {

                    /*
                    .................... User doesn't want to save .....................
                    */
                    Go_Editor(!Debug_Map);
                }
            } else {
                /*
                ** If we're in game mode, set Changed to 0 (so if we didn't save our
                ** changes above, they won't keep coming back to haunt us with continual
                ** Save Changes? prompts!)
                */
                if (!Debug_Map) {
                    Changed = 0;
                }
                Go_Editor(!Debug_Map);
            }
        }
    }

    /*------------------------------------------------------------------------
    For normal game mode, jump to the parent's AI routine.
    ------------------------------------------------------------------------*/
    if (!Debug_Map) {
        MouseClass::AI(input, x, y);
        return;
    }

    ::Frame++;

    /*------------------------------------------------------------------------
    Do special mouse processing if the mouse is over the map
    ------------------------------------------------------------------------*/
    if (Get_Mouse_X() > TacPixelX && Get_Mouse_X() < TacPixelX + Lepton_To_Pixel(TacLeptonWidth)
        && Get_Mouse_Y() > TacPixelY && Get_Mouse_Y() < TacPixelY + Lepton_To_Pixel(TacLeptonHeight)) {
        /*.....................................................................
        When the mouse moves over a scrolling edge, ScrollClass changes its
        shape to the appropriate arrow or NO symbol; it's our job to change it
        back to normal (or whatever the shape is set to by Set_Default_Mouse())
        when it re-enters the map area.
        .....................................................................*/
        if (CurTrigger || CurWaypoint != WAYPT_COUNT) {
            Override_Mouse_Shape(MOUSE_CAN_MOVE);
        } else {
            Override_Mouse_Shape(MOUSE_NORMAL);
        }
    }

    /*.....................................................................
    Set 'ZoneCell' to track the mouse cursor around over the map.  Do this
    even if the map is scrolling.
    .....................................................................*/
    if (Get_Mouse_X() >= TacPixelX && Get_Mouse_X() <= TacPixelX + Lepton_To_Pixel(TacLeptonWidth)
        && Get_Mouse_Y() >= TacPixelY && Get_Mouse_Y() <= TacPixelY + Lepton_To_Pixel(TacLeptonHeight)) {

        cell = Click_Cell_Calc(Get_Mouse_X(), Get_Mouse_Y());
        if (cell != -1) {
            Set_Cursor_Pos(cell);
            if (PendingObject) {
                Flag_To_Redraw(true);
            }
        }
    }

    /*------------------------------------------------------------------------
    Check for mouse motion while left button is down.
    ------------------------------------------------------------------------*/
    rc = Mouse_Moved();
    if (LMouseDown && rc) {
        /*.....................................................................
        "Paint" mode: place current object, and restart placement
        .....................................................................*/
        if (PendingObject && !GrabbedOverlay) {
            Flag_To_Redraw(true);
            if (Place_Object() == 0) {
                Changed = 1;

                Start_Placement();
            }
        } else if (GrabbedObject) {
            /*.....................................................................
            Move the currently-grabbed object
            .....................................................................*/
            GrabbedObject->Mark(MARK_CHANGE);
            if (Move_Grabbed_Object() == 0) {
                Changed = 1;
            }
        }
    }

    /*------------------------------------------------------------------------
    handle structure base ID textbox interactions
    ------------------------------------------------------------------------*/
    if (BaseStructureIdTextBox->Has_Focus()) {
        // if user is editing the priority textbox, prevent further input processing until they move the mouse away
        if (Get_Mouse_X() >= BaseStructureIdTextBox->X
            && Get_Mouse_X() <= BaseStructureIdTextBox->X + BaseStructureIdTextBox->Width
            && Get_Mouse_Y() >= BaseStructureIdTextBox->Y
            && Get_Mouse_Y() <= BaseStructureIdTextBox->Y + BaseStructureIdTextBox->Height) {
            input = KN_NONE;
        }

        BaseStructureIdTextBox->Draw_Me(true);
    } else if (BaseStructureIdTextBox->Has_Changed()) {
        // process AI base node priority change
        if (Change_Base_Node_Id(
            BaseStructureIdBuffer,
            std::size(BaseStructureIdBuffer),
            *BaseStructureIdTextBox,
            BaseStructureIdContext
        )) {
            BaseStructureIdContext = nullptr;
            Build_Base_To(BasePercent);
        }

        BaseStructureIdTextBox->Clear_Changed();
        BaseStructureIdTextBox->Draw_Me(true);
    }

    // give editor sidebar a change to steal input
    if (EditorSidebar.On_Input(input)) {
        // editor sidebar processed input, so finish early
        MouseClass::AI(input, x, y);
        return;
    }

    /*------------------------------------------------------------------------
    Trap special editing keys; if one is detected, set 'input' to 0 to
    prevent a conflict with parent's AI().
    ------------------------------------------------------------------------*/
    switch (input) {
    /*---------------------------------------------------------------------
    F2/RMOUSE = pop up main menu
    ---------------------------------------------------------------------*/
    case KN_RMOUSE:
        /*
        ..................... Turn off placement mode ......................
        */
        if (PendingObject) {
            if (BaseBuilding) {
                Cancel_Base_Building();
            } else {
                Cancel_Placement();
            }
            Flag_To_Redraw(true);
            break;
        }

        /*
        ................. Turn off trigger placement mode ..................
        */
        if (CurTrigger) {
            Stop_Trigger_Placement();
            Flag_To_Redraw(true);
            break;
        }

        /*
        ................. Turn off waypoint placement mode .................
        */
        if (CurWaypoint != WAYPT_COUNT) {
            Cancel_Placement();
            Flag_To_Redraw(true);
            break;
        }

        /*
        .............. Unselect object & hide popup controls ...............
        */
        if (CurrentObject.Count()) {
            CurrentObject[0]->Unselect();
            Popup_Controls();
            Flag_To_Redraw(true);
            break;
        }
        Main_Menu();
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        F6 = toggle passable/impassable display
        ---------------------------------------------------------------------*/
    case KN_F6:
        Debug_Passable = (Debug_Passable == false);
        HiddenPage.Clear();
        Flag_To_Redraw(true);
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        INSERT = go into object-placement mode
        ---------------------------------------------------------------------*/
    case KN_INSERT:
        if (!PendingObject) {
            /*
            ......... Unselect current object, hide popup controls ..........
            */
            if (CurrentObject.Count()) {
                CurrentObject[0]->Unselect();
                Popup_Controls();
            }
            /*
            .................... Go into placement mode .....................
            */
            Start_Placement();
        }
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        ESC = exit placement mode, or exit to DOS
        ---------------------------------------------------------------------*/
    case KN_ESC: {
        /*
        .................... Exit object placement mode ....................
        */
        if (PendingObject) {
            if (BaseBuilding) {
                Cancel_Base_Building();
            } else {
                Cancel_Placement();
            }
            input = KN_NONE;
            break;
        } else {

            /*
            ................... Exit trigger placement mode ....................
            */
            if (CurTrigger) {
                Stop_Trigger_Placement();
                input = KN_NONE;
                break;
            }

            /*
            ................... Exit waypoint placement mode ...................
            */
            if (CurWaypoint != WAYPT_COUNT) {
                Cancel_Placement();
                input = KN_NONE;
                break;
            }

            rc = WWMessageBox().Process("Exit Scenario Editor?", TXT_YES, TXT_NO);
            HiddenPage.Clear();
            Flag_To_Redraw(true);
            Render();

            /*
            .......... User doesn't want to exit; return to editor ..........
            */
            if (rc == 1) {
                input = KN_NONE;
                break;
            }

            /*
            ................. If changed, prompt for saving .................
            */
            if (Changed) {
                rc = WWMessageBox().Process("Save Changes?", TXT_YES, TXT_NO);
                HiddenPage.Clear();
                Flag_To_Redraw(true);
                Render();

                /*
                ..................... User wants to save .....................
                */
                if (rc == 0) {

                    /*
                    .............. If save cancelled, abort exit ..............
                    */
                    if (Save_Scenario() != 0) {
                        input = KN_NONE;
                        break;
                    } else {
                        Changed = 0;
                    }
                }
            }
        }

        Exit_Editor();
        break;
    }

        /*---------------------------------------------------------------------
        LEFT = go to previous placement object
        ---------------------------------------------------------------------*/
    case KN_LEFT:
        if (PendingObject) {
            Place_Prev();
        }
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        RIGHT = go to next placement object
        ---------------------------------------------------------------------*/
    case KN_RIGHT:
        if (PendingObject) {
            Place_Next();
        }
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        PGUP = go to previous placement category
        ---------------------------------------------------------------------*/
    case KN_PGUP:
        if (PendingObject) {
            Place_Prev_Category();
        }
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        PGDN = go to next placement category
        ---------------------------------------------------------------------*/
    case KN_PGDN:
        if (PendingObject) {
            Place_Next_Category();
        }
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        HOME = jump to first placement object, or go to Home Cell
        ---------------------------------------------------------------------*/
    case KN_HOME:
        if (PendingObject) {
            Place_Home();
        } else {

            /*
            ....................... Set map position ........................
            */
            ScenarioInit++;
            Set_Tactical_Position(Scen.Waypoint[WAYPT_HOME]);
            ScenarioInit--;

            /*
            ...................... Force map to redraw ......................
            */
            HiddenPage.Clear();
            Flag_To_Redraw(true);
            Render();
        }
        input = KN_NONE;
        break;

#if (KN_HOME | KN_SHIFT_BIT) != KN_HOME
        /*---------------------------------------------------------------------
        SHIFT-HOME: set new Home Cell position
        ---------------------------------------------------------------------*/
    case ((int)KN_HOME | (int)KN_SHIFT_BIT):
#else
    case ((int)KN_HOME | (int)KN_CTRL_BIT):
#endif
        /*
        ** Unflag the old Home Cell, if there are no other waypoints
        ** pointing to it
        */
        cell = Scen.Waypoint[WAYPT_HOME];

        if (cell != -1) {
            found = 0;
            for (i = 0; i < WAYPT_COUNT; i++) {
                if (i != WAYPT_HOME && Scen.Waypoint[i] == cell) {
                    found = 1;
                }
            }

            if (found == 0) {
                (*this)[cell].IsWaypoint = 0;
                Flag_Cell(cell);
            }
        }

        /*
        ** Now set the new Home cell
        */
        Scen.Waypoint[WAYPT_HOME] = Coord_Cell(TacticalCoord);
        (*this)[Coord_Cell(TacticalCoord)].IsWaypoint = 1;
        Flag_Cell(Coord_Cell(TacticalCoord));
        Changed = 1;
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        SHIFT-R: set new Reinforcement Cell position.  Don't allow setting
        the Reinf. Cell to the same as the Home Cell (for display purposes.)
        ---------------------------------------------------------------------*/
    case ((int)KN_R | (int)KN_SHIFT_BIT):
        if (CurrentCell == 0 || CurrentCell == Scen.Waypoint[WAYPT_HOME]) {
            break;
        }

        /*
        ** Unflag the old Reinforcement Cell, if there are no other waypoints
        ** pointing to it
        */
        cell = Scen.Waypoint[WAYPT_REINF];

        if (cell != -1) {
            found = 0;
            for (i = 0; i < WAYPT_COUNT; i++) {
                if (i != WAYPT_REINF && Scen.Waypoint[i] == cell) {
                    found = 1;
                }
            }

            if (found == 0) {
                (*this)[cell].IsWaypoint = 0;
                Flag_Cell(cell);
            }
        }
        /*
        ** Now set the new Reinforcement cell
        */
        Scen.Waypoint[WAYPT_REINF] = CurrentCell;
        (*this)[CurrentCell].IsWaypoint = 1;
        Flag_Cell(CurrentCell);
        Changed = 1;
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        ALT-Letter: Label a waypoint cell
        ---------------------------------------------------------------------*/
    case ((int)KN_A | (int)KN_ALT_BIT):
    case ((int)KN_B | (int)KN_ALT_BIT):
    case ((int)KN_C | (int)KN_ALT_BIT):
    case ((int)KN_D | (int)KN_ALT_BIT):
    case ((int)KN_E | (int)KN_ALT_BIT):
    case ((int)KN_F | (int)KN_ALT_BIT):
    case ((int)KN_G | (int)KN_ALT_BIT):
    case ((int)KN_H | (int)KN_ALT_BIT):
    case ((int)KN_I | (int)KN_ALT_BIT):
    case ((int)KN_J | (int)KN_ALT_BIT):
    case ((int)KN_K | (int)KN_ALT_BIT):
    case ((int)KN_L | (int)KN_ALT_BIT):
    case ((int)KN_M | (int)KN_ALT_BIT):
    case ((int)KN_N | (int)KN_ALT_BIT):
    case ((int)KN_O | (int)KN_ALT_BIT):
    case ((int)KN_P | (int)KN_ALT_BIT):
    case ((int)KN_Q | (int)KN_ALT_BIT):
    case ((int)KN_R | (int)KN_ALT_BIT):
    case ((int)KN_S | (int)KN_ALT_BIT):
    case ((int)KN_T | (int)KN_ALT_BIT):
    case ((int)KN_U | (int)KN_ALT_BIT):
    case ((int)KN_V | (int)KN_ALT_BIT):
    case ((int)KN_W | (int)KN_ALT_BIT):
    case ((int)KN_X | (int)KN_ALT_BIT):
    case ((int)KN_Y | (int)KN_ALT_BIT):
    case ((int)KN_Z | (int)KN_ALT_BIT):
        if (CurrentCell != 0) {
            waypt_idx = Keyboard->To_ASCII(input & 0xff) - KA_a;
            /*...............................................................
            Unflag cell for this waypoint if there is one
            ...............................................................*/
            cell = Scen.Waypoint[waypt_idx];
            if (cell != -1) {
                if (Scen.Waypoint[WAYPT_HOME] != cell && Scen.Waypoint[WAYPT_REINF] != cell)
                    (*this)[cell].IsWaypoint = 0;
                Flag_Cell(cell);
            }
            Scen.Waypoint[waypt_idx] = CurrentCell;
            (*this)[CurrentCell].IsWaypoint = 1;
            Changed = 1;
            Flag_Cell(CurrentCell);
        }
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        ALT-1-4: Designate a cell as a capture-the-flag cell.
        ---------------------------------------------------------------------*/
    case ((int)KN_1 | (int)KN_ALT_BIT):
    case ((int)KN_2 | (int)KN_ALT_BIT):
    case ((int)KN_3 | (int)KN_ALT_BIT):
    case ((int)KN_4 | (int)KN_ALT_BIT):
        /*------------------------------------------------------------------
        If there's a current cell, place the flag & waypoint there.
        ------------------------------------------------------------------*/
        if (CurrentCell != 0) {
            waypt_idx = (Keyboard->To_ASCII(input & 0xff) - KA_1);
            house = (HousesType)(HOUSE_MULTI1 + waypt_idx);
            if (HouseClass::As_Pointer(house)) {
                HouseClass::As_Pointer(house)->Flag_Attach(CurrentCell, true);
            }
        } else {
            /*------------------------------------------------------------------
            If there's a current object, attach the flag to it and clear the
            waypoint.
            ------------------------------------------------------------------*/
            if (CurrentObject[0] != 0) {
                waypt_idx = (Keyboard->To_ASCII(input & 0xff) - KA_1);
                house = (HousesType)(HOUSE_MULTI1 + waypt_idx);
                if (HouseClass::As_Pointer(house) && CurrentObject[0]->What_Am_I() == RTTI_UNIT) {
                    HouseClass::As_Pointer(house)->Flag_Attach((UnitClass*)CurrentObject[0], true);
                }
            }
        }
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        ALT-Space: Remove a waypoint designation
        ---------------------------------------------------------------------*/
    case ((int)KN_SPACE | (int)KN_ALT_BIT):
        if (CurrentCell != 0) {
            /*...............................................................
            Loop through letter waypoints; if this cell is one of them,
            clear that waypoint.
            ...............................................................*/
            for (i = 0; i < 26; i++) {
                if (Scen.Waypoint[i] == CurrentCell)
                    Scen.Waypoint[i] = -1;
            }

            /*...............................................................
            Loop through flag home values; if this cell is one of them, clear
            that waypoint.
            ...............................................................*/
            for (i = 0; i < MAX_PLAYERS; i++) {
                house = (HousesType)(HOUSE_MULTI1 + i);
                if (HouseClass::As_Pointer(house) && CurrentCell == HouseClass::As_Pointer(house)->FlagHome)
                    HouseClass::As_Pointer(house)->Flag_Remove(As_Target(CurrentCell), true);
            }

            /*...............................................................
            If there are no more waypoints on this cell, clear the cell's
            waypoint designation.
            ...............................................................*/
            if (Scen.Waypoint[WAYPT_HOME] != CurrentCell && Scen.Waypoint[WAYPT_REINF] != CurrentCell)
                (*this)[CurrentCell].IsWaypoint = 0;
            Changed = 1;
            Flag_Cell(CurrentCell);
        }
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        'H' = toggle current placement object's house
        ---------------------------------------------------------------------*/
    case KN_H:
    case ((int)KN_H | (int)KN_SHIFT_BIT):
        if (PendingObject) {
            Toggle_House();
        }
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        Left-mouse click:
        Button DOWN:
        - Toggle LMouseDown
        - If we're in placement mode, try to place the current object
          - If success, re-enter placement mode
        - Otherwise, try to select an object, and "grab" it if there is one
        - If no object, then select that cell as the "current" cell
        Button UP:
        - Toggle LMouseDown
        - release any grabbed object
        ---------------------------------------------------------------------*/
    case ((int)MAP_AREA | (int)KN_BUTTON):
        if (PopupDialogVisible
            && Get_Mouse_X() >= PopupDialogX
            && Get_Mouse_X() <= PopupDialogX + PopupDialogW
            && Get_Mouse_Y() >= PopupDialogY
            && Get_Mouse_Y() <= PopupDialogY + PopupDialogH) {
            // allow absently clicking around the popup dialog area - also required to allow edit boxes to get focus
            break;
        }

        /*
        ------------------------- Left Button DOWN -------------------------
        */
        if (Keyboard->Down(KN_LMOUSE)) {
            LMouseDown = 1;
            /*
            ............... Placement mode: place an object .................
            */
            if (PendingObject) {
                if (Place_Object() == 0) {
                    Changed = 1;

                    if (!GrabbedOverlay) {
                        Start_Placement();
                    } else {
                        GrabbedOverlay = false;
                    }
                }
            } else if (CurWaypoint != WAYPT_COUNT) {
                if (Place_Waypoint()) {
                    Cancel_Placement();
                    Flag_To_Redraw(true);
                    Changed = 1;
                }
            } else {
                /*
                ....................... Place a trigger .........................
                */
                if (CurTrigger) {
                    if (Place_Trigger()) {
                        Changed = 1;
                    }
                } else {
                    /*
                    ................. Select an object or a cell .................
                    .................. Check for double-click ....................
                    */
                    if (CurrentObject.Count() && ((WinTickCount.Time() - LastClickTime) < 15)) {
                        ; // stub

                    } else {
                        /*
                        ................ Single-click: select object .................
                        */
                        if (Select_Object() == 0) {
                            CurrentCell = 0;
                            Grab_Object();
                        } else {
                            /*
                            ................ No object: select the cell ..................
                            */
                            CurrentCell = Click_Cell_Calc(Keyboard->MouseQX, Keyboard->MouseQY);
                            HiddenPage.Clear();
                            Flag_To_Redraw(true);
                            Render();
                        }
                    }
                }
            }
            LastClickTime = WinTickCount.Time();
            input = KN_NONE;
        } else {
            /*
            -------------------------- Left Button UP --------------------------
            */
            if (GrabbedOverlay) {
                Place_Object();
                GrabbedOverlay = false;
                Changed = 1;
            }

            LMouseDown = 0;
            GrabbedObject = 0;
            input = KN_NONE;
        }
        break;

        /*---------------------------------------------------------------------
        SHIFT-ALT-Arrow: move the current object
        ---------------------------------------------------------------------*/
    case (int)KN_UP | (int)KN_ALT_BIT | (int)KN_SHIFT_BIT:
    case (int)KN_DOWN | (int)KN_ALT_BIT | (int)KN_SHIFT_BIT:
    case (int)KN_LEFT | (int)KN_ALT_BIT | (int)KN_SHIFT_BIT:
    case (int)KN_RIGHT | (int)KN_ALT_BIT | (int)KN_SHIFT_BIT:
        if (CurrentObject.Count()) {
            CurrentObject[0]->Move(KN_To_Facing(input));
            Changed = 1;
        }
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        DELETE: delete currently-selected object
        ---------------------------------------------------------------------*/
    case KN_DELETE:
        /*..................................................................
        Delete currently-selected object's trigger, or the object
        ..................................................................*/
        if (CurrentObject.Count()) {

            /*
            ........................ Delete trigger .........................
            */
            if (CurrentObject[0]->Trigger) {
                CurrentObject[0]->Trigger = NULL;
            } else {
                /*
                ** If the current object is part of the AI's Base, remove it
                ** from the Base's Node list.
                */
                if (CurrentObject[0]->What_Am_I() == RTTI_BUILDING && Base.Is_Node((BuildingClass*)CurrentObject[0])) {
                    node = Base.Get_Node((BuildingClass*)CurrentObject[0]);
                    Base.Nodes.Delete(*node);
                }

                /*
                ................... Delete current object ....................
                */
                delete CurrentObject[0];

                /*
                .................. Hide the popup controls ...................
                */
                Popup_Controls();
            }

            /*
            ........................ Force a redraw .........................
            */
            HiddenPage.Clear();
            Flag_To_Redraw(true);
            Changed = 1;
        } else {
            if (CurrentCell) {
                /*
                ................. Remove trigger from current cell .................
                */
                if ((*this)[CurrentCell].IsTrigger) {
                    (*this)[CurrentCell].IsTrigger = 0;
                    CellTriggers[CurrentCell] = NULL;
                    /*
                    ...................... Force a redraw ........................
                    */
                    HiddenPage.Clear();
                    Flag_To_Redraw(true);
                    Changed = 1;
                } else {
                    auto& current_cell = Array[CurrentCell];

                    // first try to delete overlay (if any)
                    if (!current_cell.Purge_Overlay()) {
                        // purge any object in this cell, most objects are grabbable so this is an edge case
                        if (current_cell.Cell_Occupier() != nullptr) {
                            current_cell.Cell_Occupier()->Delete_This();
                            Flag_To_Redraw(true);
                        }
                    } else {
                        Flag_To_Redraw(true);
                    }
                }
            }
        }
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        TAB: select next object on the map
        ---------------------------------------------------------------------*/
    case KN_TAB:
        Select_Next();
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        Object-Editing button: House Button
        ---------------------------------------------------------------------*/
    case (POPUP_GDI | KN_BUTTON):
    case (POPUP_NOD | KN_BUTTON):
    case (POPUP_NEUTRAL | KN_BUTTON):
    case (POPUP_MULTI1 | KN_BUTTON):
    case (POPUP_MULTI2 | KN_BUTTON):
    case (POPUP_MULTI3 | KN_BUTTON):
    case (POPUP_MULTI4 | KN_BUTTON):
        /*..................................................................
        Convert input value into a house value; assume HOUSE_GOOD is 0
        ..................................................................*/
        house = (HousesType)((input & (~KN_BUTTON)) - POPUP_GDI);
        /*..................................................................
        If that house doesn't own this object, try to transfer it
        ..................................................................*/
        if (CurrentObject[0]->Owner() != house) {
            if (Change_House(house)) {
                Changed = 1;
            }
        }
        Set_House_Buttons(CurrentObject[0]->Owner(), Buttons, POPUP_GDI);
        HiddenPage.Clear();
        Flag_To_Redraw(true);
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        Object-Editing button: Mission
        ---------------------------------------------------------------------*/
    case (POPUP_MISSIONLIST | KN_BUTTON):
        if (CurrentObject[0]->Is_Techno()) {
            /*
            ........................ Set new mission ........................
            */
            mission = MapEditMissions[MissionList->Current_Index()];
            if (CurrentObject[0]->Get_Mission() != mission) {
                ((TechnoClass*)CurrentObject[0])->Set_Mission(mission);
                Changed = 1;
            }
        }
        Flag_To_Redraw(true);
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        Object-Editing button: Health
        ---------------------------------------------------------------------*/
    case (POPUP_HEALTHGAUGE | KN_BUTTON):
        if (CurrentObject[0]->Is_Techno()) {
            /*
            .......... Derive strength from current gauge reading ...........
            */
            strength = Fixed_To_Cardinal((unsigned)CurrentObject[0]->Class_Of().MaxStrength,
                                         (unsigned)HealthGauge->Get_Value());

            /*
            ........................... Clip to 1 ...........................
            */
            if (strength <= 0) {
                strength = 1;
            }

            /*
            ....................... Set new strength ........................
            */
            if (strength != CurrentObject[0]->Strength) {
                CurrentObject[0]->Strength = strength;

                if (CurrentObject[0]->What_Am_I() == RTTI_BUILDING) {
                    reinterpret_cast<BuildingClass*>(CurrentObject[0])->LastStrength = strength;
                }

                HiddenPage.Clear();
                Flag_To_Redraw(true);
                Changed = 1;
            }

            /*
            ....................... Update text label .......................
            */
            sprintf(HealthBuf, "%d", strength);
        }
        input = KN_NONE;
        break;

        /*---------------------------------------------------------------------
        Object-Editing button: Facing
        ---------------------------------------------------------------------*/
    case (POPUP_FACINGDIAL | KN_BUTTON):
        if (CurrentObject[0]->Is_Techno()) {
            /*
            ........................ Set new facing .........................
            */
            if (FacingDial->Get_Direction() != ((TechnoClass*)CurrentObject[0])->PrimaryFacing.Get()) {
                /*
                ..................... Set body's facing ......................
                */
                ((TechnoClass*)CurrentObject[0])->PrimaryFacing.Set(FacingDial->Get_Direction());

                /*
                ............. Set turret facing, if there is one .............
                */
                if (CurrentObject[0]->What_Am_I() == RTTI_UNIT) {
                    ((UnitClass*)CurrentObject[0])->SecondaryFacing.Set(FacingDial->Get_Direction());
                }

                HiddenPage.Clear();
                Flag_To_Redraw(true);
                Changed = 1;
            }
        }
        input = KN_NONE;
        break;

    // when the 'Base' checkbox is toggled, add/remove the selected building (if any) from the AI base
    case (POPUP_BASESTRUCTURE | KN_BUTTON): {
        input = KN_NONE;

        if (CurrentObject.Count() < 1 || CurrentObject[0]->What_Am_I() != RTTI_BUILDING) {
            break;
        }

        auto selected_building = dynamic_cast<BuildingClass*>(CurrentObject[0]);
        const auto building_strength = selected_building->Strength;
        const auto facing = selected_building->PrimaryFacing;
        const auto trigger = selected_building->Trigger;

        auto base_changed = false;

        BaseNodeClass building_node;

        building_node.Type = selected_building->Class->Type;
        building_node.Coord = selected_building->Coord;

        if (!IsBaseStructureCheckbox->IsOn) {
            if (!Base.Is_Node(selected_building)) {
                Base.Nodes.Add(building_node);
                base_changed = true;
            }

            IsBaseStructureCheckbox->Turn_On();
            BaseStructureIdText->Enable();
            BaseStructureIdTextBox->Enable();
        } else if (IsBaseStructureCheckbox->IsOn) {
            if (Base.Is_Node(selected_building)) {
                Base.Nodes.Delete(building_node);
                base_changed = true;
            }

            IsBaseStructureCheckbox->Turn_Off();
            BaseStructureIdTextBox->Disable(true);
            BaseStructureIdText->Disable(true);
        }

        if (base_changed) {
            Changed = 1;

            // rebuild AI base to reflect change
            Build_Base_To(BasePercent);

            // restore building state (adding existing structure to base causes it to be recreated)
            auto new_building = Base.Get_Building(building_node);

            if (new_building != nullptr) {
                new_building->Strength = building_strength;
                new_building->LastStrength = building_strength;
                new_building->PrimaryFacing = facing;

                // note: triggers are blown away if base percent causes the building to not be a starting structure
                new_building->Trigger = trigger;

                new_building->Select();
                new_building->Time_To_Redraw();

                if (Base.Is_Node(new_building)) {
                    sprintf(BaseStructureIdBuffer, "%d", Base.Nodes.ID(Base.Get_Node(new_building)));
                    BaseStructureIdTextBox->Set_Text(BaseStructureIdBuffer, std::size(BaseStructureIdBuffer));
                    BaseStructureIdContext = new_building;
                }
            }

            Flag_To_Redraw(true);
        }

        break;
    }

    /*---------------------------------------------------------------------
    Object-Editing button: Facing
    ---------------------------------------------------------------------*/
    case (POPUP_BASEPERCENT | KN_BUTTON):
        if (BaseGauge->Get_Value() != BasePercent) {
            BasePercent = BaseGauge->Get_Value();
            Build_Base_To(BasePercent);
            HiddenPage.Clear();
            Flag_To_Redraw(true);
        }
        input = KN_NONE;
        break;

    case (KN_LMOUSE):
        input = KN_NONE;
        break;

    default:
        break;
    }

    /*
    ------------------------ Call parent's AI routine ------------------------
    */
    MouseClass::AI(input, x, y);
}

static int Calculate_Power_For_House(const HousesType house)
{
    auto power = 0;

    for (auto i = 0; i < Buildings.Count(); i++) {
        const auto building = Buildings.Ptr(i);

        // only count building if it belongs to the requested house and isn't a 'virtual' AI base building
        if (building == nullptr || building->Owner() != house || building->IsUnbuiltBase) {
            continue;
        }

        // replicates BuildingClass::Power_Output() logic without relying on LastStrength field
        // (updated by AI loop, but that doesn't run in scenario editor mode)
        if (building->Class->Power) {
            power += static_cast<int>(
                Fixed_To_Cardinal(
                    building->Class->Power,
                    Cardinal_To_Fixed(
                        building->Class->MaxStrength,
                        building->Strength
                    )
                )
            );
        }

        power -= building->Class->Drain;
    }

    return power;
}

void MapEditClass::Draw_Footer(const bool forced)
{
    LogicPage->Fill_Rect(FooterX, FooterY, FooterX + FooterW, FooterY + FooterH, BLACK);
    LogicPage->Draw_Line(FooterX, FooterY - 1, FooterX + FooterW, FooterY - 1, GRAY);

    /*
    **	Power output display.
    */
    constexpr auto fore = GREEN;
    constexpr auto back = TBLACK;
    static const auto text_flags = TPF_NOSHADOW | TPF_6PT_GRAD | TPF_USE_GRAD_PAL;

    static const auto power_to_color = [](const int power) {
        return power == 0
            ? GRAY
            : power > 0
                ? GREEN
                : RED;
    };

    auto label_x = FooterX + 5u;

    if (GameToPlay == GAME_NORMAL) {
        // power readouts for GDI/NOD
        const auto gdi_label = std::format("{} Power: ", Text_String(TXT_G_D_I));
        const auto nod_label = std::format(" | {} Power: ", Text_String(TXT_N_O_D));

        const auto gdi_power = Calculate_Power_For_House(HOUSE_GOOD);
        const auto gdi_power_str = std::format("{}", gdi_power);

        const auto nod_power = Calculate_Power_For_House(HOUSE_BAD);
        const auto nod_power_str = std::format("{}", nod_power);

        // gdi power
        Fancy_Text_Print(gdi_label.c_str(), label_x , FooterY, fore, back, text_flags);
        label_x += String_Pixel_Width(gdi_label.c_str());

        Fancy_Text_Print(gdi_power_str.c_str(), label_x , FooterY, power_to_color(gdi_power), back, text_flags);
        label_x += String_Pixel_Width(gdi_power_str.c_str());

        // nod power
        Fancy_Text_Print(nod_label.c_str(), label_x , FooterY, fore, back, text_flags);
        label_x += String_Pixel_Width(nod_label.c_str());

        Fancy_Text_Print(nod_power_str.c_str(), label_x , FooterY, power_to_color(nod_power), back, text_flags);
        label_x += String_Pixel_Width(nod_power_str.c_str());

        Fancy_Text_Print(" | ", label_x , FooterY, fore, back, text_flags);
        label_x += String_Pixel_Width(" | ");
    } else {
        // TODO: consider multi support?
    }

    // power readout neutral
    constexpr auto neutral_label = "Neutral Power: ";

    const auto neutral_power = Calculate_Power_For_House(HOUSE_NEUTRAL);
    const auto neutral_power_str = std::format("{}", neutral_power);

    // neutral power
    Fancy_Text_Print(neutral_label, label_x , FooterY, fore, back, text_flags);
    label_x += String_Pixel_Width(neutral_label);

    Fancy_Text_Print(neutral_power_str.c_str(), label_x , FooterY, power_to_color(neutral_power), back, text_flags);

    /*
    **	Draw tracker for currently selected object/cell location.
    */
    static const auto wide_location_display = SeenBuff.Get_Width() > GBUFF_INIT_WIDTH + 100;

    const auto target_cell = CurrentObject.Count() > 0 ? Coord_Cell(CurrentObject[0]->Coord) : CurrentCell;
    const auto cell_x = Cell_X(target_cell);
    const auto cell_y = Cell_Y(target_cell);
    const auto cell_coord = XY_Coord(cell_x, cell_y) * 256; // scale coord to match INI file format

    auto cell_number = Array[target_cell].Cell_Number();

#ifdef MEGAMAPS
    // if we are editing a non-megamap scenario using a megamap build, adjust the cell_number to display
    // what will be seen in the INI file on save
    if (MapBinaryVersion == MAP_VERSION_NORMAL && cell_x <= 64 && cell_y <= 64) {
        const auto unconfined_cell = Unconfine_Old_Cell(target_cell);

        cell_number = Array[unconfined_cell].Cell_Number();
    }
#endif

    Fancy_Text_Print(
        wide_location_display ? "| Coord %u - Cell #%d @ %dx%d" : "| Coord %u - Cell"
                                                                  " #%d",
        ((FooterX + (FooterW - (FooterW / 3))) + 5 ) - (wide_location_display ? 0 : 30),
        FooterY,
        CC_GREEN,
        TBLACK,
        TPF_NOSHADOW | TPF_6PT_GRAD | TPF_USE_GRAD_PAL,
        cell_coord,
        cell_number,
        cell_x,
        cell_y
    );
}

void MapEditClass::Decorate_Cells(const bool forced)
{
    // TODO: have settings dialog menu options for font/colours in scenario editor
    static auto constexpr cell_text_back_color = TBLACK;
    static const auto cell_text_flags = TPF_FULLSHADOW | TPF_8POINT | TPF_CENTER;
    const auto trigger_color = TdSettings.Get_Editor_Trigger_Color();
    const auto waypoint_colour = TdSettings.Get_Editor_Waypoint_Color();

    Iterate_Over_Map_Cells(
        [&](auto raw_cell, auto& cell) {
            auto cell_object = cell.Cell_Occupier();

            int x, y;
            if (!Coord_To_Pixel(cell.Cell_Coord(), x, y)) {
                // failed to resolve cell screen co-ords
                return;
            }

            if (!In_View(raw_cell)) {
                // don't decorate a non-visible cell
                return;
            }

            auto render_x = x + TacPixelX;
            auto render_y = y + TacPixelY - (CELL_PIXEL_H / 4);

            if (render_y < HeaderY + HeaderH + 1 || render_y > FooterY - 1) {
                // prevent rendering over header/footer areas
                return;
            }

            if (cell.IsTrigger) {
                /*
                **	Draw the cell's Trigger mnemonic, if it has a trigger
                */
                const auto& trig = *cell.Get_Trigger();

                Fancy_Text_Print(
                    trig.Get_Name(),
                    render_x,
                    render_y,
                    trigger_color,
                    cell_text_back_color,
                    cell_text_flags
                );
            } else if (cell.IsWaypoint) {
                /*
                **	Draw the cell's Waypoint designation if there is one.
                */
                for (auto i = 0; i < WAYPT_HOME; i++) {
                    if (Scen.Waypoint[i] == cell.Cell_Number()) {
                        if (GameToPlay != GAME_NORMAL && i < MPlayerMax && !cell.IsFlagged) {
                            // mark waypoints as multiplayer start positions using flags
                            cell.Flag_Place(HOUSE_NONE);
                            cell.Draw_It(x + TacPixelX, y + TacPixelY);
                        }

                        Fancy_Text_Print(std::format("{}", i).c_str(),
                                         render_x,
                                         render_y,
                                         waypoint_colour,
                                         cell_text_back_color,
                                         cell_text_flags);
                        break;
                    }
                }

                if (Scen.Waypoint[WAYPT_HOME] == cell.Cell_Number()) {
                    Fancy_Text_Print("HOME",
                                     render_x,
                                     render_y,
                                     waypoint_colour,
                                     cell_text_back_color,
                                     cell_text_flags);
                }

                if (Scen.Waypoint[WAYPT_REINF] == cell.Cell_Number()) {
                    Fancy_Text_Print("REINF",
                                     render_x,
                                     render_y,
                                     waypoint_colour,
                                     cell_text_back_color,
                                     cell_text_flags);
                }
            } else if (cell_object != nullptr && cell_object->Trigger) {
                if (cell_object->What_Am_I() == RTTI_BUILDING && cell.Cell_Building()->IsUnbuiltBase) {
                    // hide triggers stored in unbuilt buildings
                    // (they are only cached in-case the building becomes a starting structure again)
                    return;
                }

                // draw object trigger (building/unit/infantry etc.)
                const auto coord = cell_object->Render_Coord();
                int object_x, object_y;

                if (Coord_To_Pixel(coord, object_x, object_y)) {
                    Fancy_Text_Print(
                        cell_object->Trigger->Get_Name(),
                        object_x + (WinX << 3),
                        object_y + (CELL_PIXEL_H / 2),
                        trigger_color,
                        cell_text_back_color,
                        cell_text_flags
                    );
                }
            }
        }
    );
}

void MapEditClass::Draw_Header(const bool forced)
{
    const auto factor = SeenBuff.Get_Width() == GBUFF_INIT_WIDTH / 2 ? 1 : 2;

    LogicPage->Fill_Rect(HeaderX, HeaderY, HeaderX + HeaderW, HeaderY + HeaderH, BLACK);
    LogicPage->Draw_Line(HeaderX, HeaderY + HeaderH  + 1, HeaderX + HeaderW, HeaderY + HeaderH + 1, GRAY);

    // total value of all Tiberium on the map
    Fancy_Text_Print(
        "Tiberium=%ld   ", HeaderX, HeaderY, CC_GREEN, BLACK, TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW, TotalValue);

    // name of the scenario being edited
    std::string scenario_title = Scen.ScenarioName;

    if (Scen.ScenarioBasicName.has_value()) {
        scenario_title += " - ";
        scenario_title += *Scen.ScenarioBasicName;
    }

    Fancy_Text_Print(
        scenario_title.c_str(),
        (HeaderX + HeaderW / 2),
        HeaderY,
        CC_TAN,
        TBLACK,
        TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD | TPF_USE_GRAD_PAL
    );

    // exact AI base percent value (beside associated slider)
    Fancy_Text_Print(
        "%3d%%", HeaderX + (HeaderW - (22 * factor)), HeaderY, CC_GREEN, BLACK, TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW, BasePercent);
}

void MapEditClass::Render_Editor_Controls()
{
    if (Buttons == nullptr) {
        return;
    }

    EditorSidebar.Render();

    // dynamically determine if popup dialog is visible
    PopupDialogVisible = false;

    const auto map_midpoint_y = (SeenBuff.Get_Height() - Map.Get_Tab_Height()) / 2;
    auto control = Buttons;

    while (control != nullptr) {
        // popup buttons are only after the midpoint and don't overlap the sidebar, so use that as a heuristic to
        // detect a popup dialog
        if (control->Y >= map_midpoint_y && control->X < EditorSidebar.X - 1) {
            PopupDialogVisible = true;
            break;
        }

        control = control->Get_Next();
    }

    if (PopupDialogVisible) {
        // draw a dialog background for the popup buttons
        Dialog_Box(PopupDialogX, PopupDialogY, PopupDialogW, PopupDialogH);

        // print the selected object name
        if (CurrentObject.Count() > 0) {
            // init font
            Fancy_Text_Print(
                TXT_NONE,
                0,
                0,
                CC_GREEN,
                TBLACK,
                TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW
            );
            Draw_Caption(
                Text_String(CurrentObject[0]->Full_Name()),
                OPTION_DIALOG,
                PopupDialogX,
                PopupDialogY,
                PopupDialogW
            );
        }
    }

    Buttons->Draw_All();
}

/***************************************************************************
 * MapEditClass::Draw_It -- overloaded Redraw routine                      *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/17/1994 BR : Created.                                              *
 *=========================================================================*/
void MapEditClass::Draw_It(bool forced)
{
    char const* label;
    char buf[40];
    char const* tptr;

    MouseClass::Draw_It(forced);

    if (!Debug_Map) {
        return;
    }

    Draw_Header(forced);
    Decorate_Cells(forced);
    Draw_Footer(forced);
}

/***************************************************************************
 * MapEditClass::Mouse_Moved -- checks for mouse motion                    *
 *                                                                         *
 * Reports whether the mouse has moved or not. This varies based on the    *
 * type of object currently selected. If there's an infantry object        *
 *   selected, mouse motion counts even within a cell; for all other types,*
 *   mouse motion counts only if the mouse changes cells.                  *
 *                                                                         *
 *   The reason this routine is needed is to prevent Paint-Mode from putting*
 *   gobs of trees and such into the same cell if the mouse moves just     *
 *   a little bit.                                                         *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/08/1994 BR : Created.                                              *
 *=========================================================================*/
bool MapEditClass::Mouse_Moved(void)
{
    static int old_mx = 0;
    static int old_my = 0;
    static CELL old_zonecell = 0;
    const ObjectTypeClass* objtype = NULL;
    bool retcode = false;

    /*
    -------------------------- Return if no motion ---------------------------
    */
    if (old_mx == Get_Mouse_X() && old_my == Get_Mouse_Y()) {
        return (false);
    }

    /*
    ---------------------- Get a ptr to ObjectTypeClass ----------------------
    */
    if (PendingObject) {
        objtype = PendingObject;
    } else {
        if (GrabbedObject) {
            objtype = &GrabbedObject->Class_Of();
        } else {
            old_mx = Get_Mouse_X();
            old_my = Get_Mouse_Y();
            old_zonecell = ZoneCell;
            return (false);
        }
    }

    /*
    --------------------- Check for motion based on type ---------------------
    */
    /*
    ............... Infantry: mouse moved if any motion at all ...............
    */
    if (objtype->What_Am_I() == RTTI_INFANTRYTYPE) {
        retcode = true;
    } else {
        /*
        ................ Others: mouse moved only if cell changed ................
        */
        if (old_zonecell != ZoneCell) {
            retcode = true;
        } else {
            retcode = false;
        }
    }

    old_mx = Get_Mouse_X();
    old_my = Get_Mouse_Y();
    old_zonecell = ZoneCell;
    return (retcode);
}

/***************************************************************************
 * MapEditClass::Main_Menu -- main menu processor for map editor           *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/20/1994 BR : Created.                                              *
 *=========================================================================*/
void MapEditClass::Main_Menu(void)
{
    char const* _menus[MAX_MAIN_MENU_NUM + 1];
    int selection; // option the user picks
    bool process;  // menu stays up while true
    int rc;

    /*
    --------------------------- Fill in menu items ---------------------------
    */
    _menus[0] = "New Scenario";
    _menus[1] = "Load Scenario";
    _menus[2] = "Save Scenario";
    _menus[3] = "Size Map";
    _menus[4] = "Add Game Object";
    _menus[5] = "Scenario Options";
    _menus[6] = "AI Options";
    _menus[7] = "Play Scenario";
    _menus[8] = "Exit";
    _menus[9] = NULL;

    /*
    ----------------------------- Main Menu loop -----------------------------
    */
    Override_Mouse_Shape(MOUSE_NORMAL); // display default mouse cursor
    process = true;
    while (process) {

        /*
        ................ Invoke game callback, to update music ................
        */
        Call_Back();

        /*
        ............................. Invoke menu .............................
        */
        Hide_Mouse(); // Do_Menu assumes the mouse is already hidden
        selection = Do_Menu(&_menus[0], true);
        Show_Mouse();
        if (UnknownKey == KN_ESC || UnknownKey == KN_LMOUSE || UnknownKey == KN_RMOUSE) {
            break;
        }

        /*
        .......................... Process selection ..........................
        */
        switch (selection) {
        /*
        ........................... New scenario ...........................
        */
        case 0:
            if (Changed) {
                rc = WWMessageBox().Process("Save Changes?", TXT_YES, TXT_NO);
                HiddenPage.Clear();
                Flag_To_Redraw(true);
                Render();
                if (rc == 0) {
                    if (Save_Scenario() != 0) {
                        break;
                    } else {
                        Changed = 0;
                    }
                }
            }
            if (New_Scenario() == 0) {
                Scen.CarryOverMoney = 0;
                Changed = 1;
            }
            process = false;
            break;

        /*
        .......................... Load scenario ...........................
        */
        case 1:
            if (Changed) {
                rc = WWMessageBox().Process("Save Changes?", TXT_YES, TXT_NO);
                HiddenPage.Clear();
                Flag_To_Redraw(true);
                Render();
                if (rc == 0) {
                    if (Save_Scenario() != 0) {
                        break;
                    } else {
                        Changed = 0;
                    }
                }
            }

            if (Mission_Select_Dialog()) {
                Flag_To_Redraw(true);
                Render();

                if (ScenPlayer != SCEN_PLAYER_MPLAYER && ScenPlayer != SCEN_PLAYER_JP) {
                    LastHouse = HOUSE_GOOD;
                }

                Scen.CarryOverMoney = 0;
                Build_Base_To(BasePercent);
                Changed = 0;
            }
            process = false;
            break;

        /*
        .......................... Save scenario ...........................
        */
        case 2:
            if (Save_Scenario() == 0) {
                Changed = 0;
            }
            process = false;
            break;

        /*
        .......................... Edit map size ...........................
        */
        case 3:
            if (Size_Map(IniMapCellX, IniMapCellY, IniMapCellWidth, IniMapCellHeight) == 0) {
                process = false;
                Changed = 1;
            }
            break;

        /*
        .......................... Add an object ...........................
        */
        case 4:
            if (Placement_Dialog() == 0) {
                Start_Placement();
                process = false;
            }
            break;

        /*
        ......................... Scenario options .........................
        */
        case 5:
            if (Scenario_Dialog() == 0) {
                Changed = 1;
                process = false;
            }
            break;

        /*
        .......................... Other options ...........................
        */
        case 6:
            AI_Menu();
            process = false;
            break;

        /*
        ...................... Test-drive this scenario ....................
        */
        case 7:
            if (Changed) {
                rc = WWMessageBox().Process("Save Changes?", TXT_YES, TXT_NO);
                HiddenPage.Clear();
                Flag_To_Redraw(true);
                Render();
                if (rc == 0) {
                    if (Save_Scenario() != 0) {
                        break;
                    } else {
                        Changed = 0;
                    }
                }
            }
            Changed = 0;
            Debug_Map = false;
            Debug_Unshroud = false;

            Start_Scenario(Scen.ScenarioName);

            // reset view dimensions, prevents issues with tactical map position and shroud
            Set_View_Dimensions(0, Map.Get_Tab_Height());

            return;

        case 8:
            Exit_Editor();
            return;
        }

        Frame_Limiter();
    }

    /*------------------------------------------------------------------------
    Restore the display:
    - Clear HIDPAGE to erase any spurious drawing done by the menu system
    - Invoke Flag_To_Redraw to tell DisplayClass to re-render the whole screen
    - Invoke Redraw() to update the display
    ------------------------------------------------------------------------*/
    HiddenPage.Clear();
    Flag_To_Redraw(true);
    Render();
}

/***************************************************************************
 * MapEditClass::AI_Menu -- menu of AI options                             *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/29/1994 BR : Created.                                              *
 *=========================================================================*/
void MapEditClass::AI_Menu(void)
{
    int selection; // option the user picks
    bool process;  // menu stays up while true
    char const* _menus[MAX_AI_MENU_NUM + 1];

    /*
    -------------------------- Fill in menu strings --------------------------
    */
    _menus[0] = "Pre-Build a Base";
    _menus[1] = "Import Triggers";
    _menus[2] = "Edit Triggers";
    _menus[3] = "Import Teams";
    _menus[4] = "Edit Teams";
    _menus[5] = NULL;

    /*
    ----------------------------- Main Menu loop -----------------------------
    */
    Override_Mouse_Shape(MOUSE_NORMAL); // display default mouse cursor
    process = true;
    while (process) {

        /*
        ................ Invoke game callback, to update music ................
        */
        Call_Back();

        /*
        ............................. Invoke menu .............................
        */
        Hide_Mouse(); // Do_Menu assumes the mouse is already hidden
        selection = Do_Menu(&_menus[0], true);
        Show_Mouse();
        if (UnknownKey == KN_ESC || UnknownKey == KN_LMOUSE || UnknownKey == KN_RMOUSE) {
            break;
        }

        /*
        .......................... Process selection ..........................
        */
        switch (selection) {
        /*
        ......................... Pre-Build a Base .........................
        */
        case 0:
            Start_Base_Building();
            process = false;
            break;

        /*
        ......................... Import Triggers ..........................
        */
        case 1:
            if (Import_Triggers() == 0)
                process = false;
            break;

        /*
        ......................... Trigger Editing ..........................
        */
        case 2:
            Handle_Triggers();
            /*
            ................ Go into trigger placement mode .................
            */
            if (CurTrigger) {
                Start_Trigger_Placement();
            }
            process = false;
            break;

        /*
        ........................... Import Teams ...........................
        */
        case 3:
            if (Import_Teams() == 0)
                process = false;
            break;

        /*
        ........................... Team Editing ...........................
        */
        case 4:
            Handle_Teams("Teams");
            process = false;
            break;
        }

        Frame_Limiter();
    }
}

/***************************************************************************
 * MapEditClass::Verify_House -- is this objtype ownable by this house?    *
 *                                                                         *
 * INPUT:                                                                  *
 *      house         house to check                                       *
 *      objtype      ObjectTypeClass to check                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      0 = isn't ownable, 1 = it is                                       *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/16/1994 BR : Created.                                              *
 *=========================================================================*/
bool MapEditClass::Verify_House(HousesType house, ObjectTypeClass const* objtype)
{
    if (!TdSettings.Enforce_OwnableBy_In_Editor()) {
        return true;
    }

    /*
    --------------- Verify that new house can own this object ----------------
    */
    return ((objtype->Get_Ownable() & (1 << house)) != 0);
}

/***************************************************************************
 * MapEditClass::Cycle_House -- finds next valid house for object type     *
 *                                                                         *
 * INPUT:                                                                  *
 *      objtype      ObjectTypeClass ptr to get house for                  *
 *      curhouse      current house value to start with                    *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      HousesType that's valid for this object type                       *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/23/1994 BR : Created.                                              *
 *=========================================================================*/
HousesType MapEditClass::Cycle_House(HousesType curhouse, ObjectTypeClass const* objtype)
{
    HousesType count; // prevents an infinite loop

    /*------------------------------------------------------------------------
    Loop through all house types, starting with the one after 'curhouse';
    return the first one that's valid
    ------------------------------------------------------------------------*/
    count = HOUSE_NONE;
    while (1) {

        /*
        .......................... Go to next house ...........................
        */
        curhouse++;
        if (curhouse == HOUSE_COUNT) {
            curhouse = HOUSE_FIRST;
        }

        /*
        ................ Count # iterations; don't go forever .................
        */
        count++;
        if (count == HOUSE_COUNT) {
            curhouse = HOUSE_NONE;
            break;
        }

        /*
        ................... Break if this is a valid house ....................
        */
        if (HouseClass::As_Pointer(curhouse) && Verify_House(curhouse, objtype)) {
            break;
        }
    }

    return (curhouse);
}

/***************************************************************************
 * MapEditClass::Fatal -- exits with error message                         *
 *                                                                         *
 * INPUT:                                                                  *
 *      code      tells which message to display; this minimizes the       *
 *               use of character strings in the code.                     *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/12/1994 BR : Created.                                              *
 *=========================================================================*/
void MapEditClass::Fatal(int txt)
{
    Prog_End();
    printf("%s\n", Text_String(txt));
    if (!RunningAsDLL) {
        exit(EXIT_FAILURE);
    }
}

bool MapEditClass::Scroll_Map(DirType facing, int& distance, bool really)
{
    if (Debug_Map) {
        /*
        ** The popup gadgets require the entire map to be redrawn if we scroll.
        */
        if (really) {
            Flag_To_Redraw(true);
        }
    }
    return (MouseClass::Scroll_Map(facing, distance, really));
}

void MapEditClass::Detach(ObjectClass* object)
{
    if (GrabbedObject == object) {
        GrabbedObject = 0;
    }
}

void MapEditClass::Init_Editor_Dimensions()
{
    Set_View_Dimensions(
        0,
        HeaderY + HeaderH + 2,
        SeenBuff.Get_Width() - EditorSidebar.W - 1,
        EditorSidebar.H
    );
}

#endif
