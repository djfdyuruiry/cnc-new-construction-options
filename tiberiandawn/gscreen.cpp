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

/* $Header:   F:\projects\c&c\vcs\code\gscreen.cpv   2.17   16 Oct 1995 16:51:34   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  W E S T W O O D   S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : GSCREEN.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 12/15/94                                                     *
 *                                                                                             *
 *                  Last Update : January 19, 1995 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   GScreenClass::GScreenClass -- Default constructor for GScreenClass.                       *
 *   GScreenClass::One_Time -- Handles one time class setups.                                  *
 *   GScreenClass::Init -- Init's the entire display hierarchy by calling all Init routines.   *
 *   GScreenClass::Init_Clear -- Sets the map to a known state.                                *
 *   GScreenClass::Init_Theater -- Performs theater-specific initializations.                  *
 *   GScreenClass::Init_IO -- Initializes the Button list ('Buttons').                         *
 *   GScreenClass::Flag_To_Redraw -- Flags the display to be redrawn.                          *
 *   GScreenClass::Blit_Display -- Redraw the display from the hidpage to the seenpage.        *
 *   GScreenClass::Render -- General drawing dispatcher an display update function.            *
 *   GScreenClass::Input -- Fetches input and processes gadgets.                               *
 *   GScreenClass::Add_A_Button -- Add a gadget to the game input system.                      *
 *   GScreenClass::Remove_A_Button -- Removes a gadget from the game input system.             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include <cmath>

#include "function.h"

#include "common/filepcx.h"

GadgetClass* GScreenClass::Buttons = 0;

/***********************************************************************************************
 * GScreenClass::GScreenClass -- Default constructor for GScreenClass.                         *
 *                                                                                             *
 *    This constructor merely sets the display system, so that it will redraw the first time   *
 *    the render function is called.                                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/15/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
GScreenClass::GScreenClass(void)
{
    IsToUpdate = true;
    IsToRedraw = true;
    VisibleRallyPointSource = nullptr;
}

/***********************************************************************************************
 * GScreenClass::One_Time -- Handles one time class setups.                                    *
 *                                                                                             *
 * This routine (and all those that overload it) must perform truly one-time initialization.   *
 * Such init's would normally be done in the constructor, but other aspects of the game may    *
 * not have been initialized at the time the constructors are called (such as the file system, *
 * the display, or other WWLIB subsystems), so many initializations should be deferred to the  *
 * One_Time init's.                                                                            *
 *                                                                                             *
 * Any variables set in this routine should be declared as static, so they won't be modified   *
 * by the load/save process.  Non-static variables will be over-written by a loaded game.      *
 *                                                                                             *
 * This function allocates the shadow buffer that is used for quick screen updates. If         *
 * there were any data files to load, they would be loaded at this time as well.               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Call this routine only ONCE at the beginning of the game.                       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/15/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void GScreenClass::One_Time(void)
{
    Buttons = 0;
    HiddenPage.Clear();
}

/***********************************************************************************************
 * GScreenClass::Init -- Init's the entire display hierarchy by calling all Init routines.     *
 *                                                                                             *
 * This routine shouldn't be overloaded.  It's the main map initialization routine, and will   *
 * perform a complete map initialization, from mixfiles to clearing the buffers.  Calling this *
 * routine results in calling every initialization routine in the entire map hierarchy.        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      theater      theater to initialize to                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/28/1994 BR : Created.                                                                  *
 *=============================================================================================*/
void GScreenClass::Init(TheaterType theater)
{
    Init_Clear();
    Init_IO();
    Init_Theater(theater);
}

/***********************************************************************************************
 * GScreenClass::Init_Clear -- Sets the map to a known state.                                  *
 *                                                                                             *
 * This routine (and those that overload it) clears any buffers and variables to a known       *
 * state.  It assumes that all buffers are allocated & valid.  The map should be displayable   *
 * after calling this function, and should draw basically an empty display.                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/28/1994 BR : Created.                                                                  *
 *=============================================================================================*/
void GScreenClass::Init_Clear(void)
{
    /*
    ** Clear the HidPage to force a complete shadow blit.
    */
    HiddenPage.Clear();
    IsToRedraw = true;
    VisibleRallyPointSource = nullptr;
}

/***********************************************************************************************
 * GScreenClass::Init_Theater -- Performs theater-specific initializations.                    *
 *                                                                                             *
 * This routine (and those that overload it) performs any theater-specific initializations     *
 * needed.  This will include setting the palette, setting up remap tables, etc.  This routine *
 * only needs to be called when the theater has changed.                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/28/1994 BR : Created.                                                                  *
 *=============================================================================================*/
void GScreenClass::Init_Theater(TheaterType)
{
}

/***********************************************************************************************
 * GScreenClass::Init_IO -- Initializes the Button list ('Buttons').                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/28/1994 BR : Created.                                                                  *
 *=============================================================================================*/
void GScreenClass::Init_IO(void)
{
    /*
    ** Reset the button list.  This means that any other elements of the map that need
    ** buttons must attach them after this routine is called!
    */
    Buttons = 0;
}

/***********************************************************************************************
 * GScreenClass::Flag_To_Redraw -- Flags the display to be redrawn.                            *
 *                                                                                             *
 *    This function is used to flag the display system whether any rendering is needed. The    *
 *    parameter tells the system either to redraw EVERYTHING, or just that something somewhere *
 *    has changed and the individual Draw_It functions must be called. When a sub system       *
 *    determines that it needs to render something local to itself, it would call this routine *
 *    with a false parameter. If the entire screen gets trashed or needs to be rebuilt, then   *
 *    this routine will be called with a true parameter.                                       *
 *                                                                                             *
 * INPUT:   complete -- bool; Should the ENTIRE screen be redrawn?                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This doesn't actually draw the screen, it merely sets flags so that when the    *
 *             Render() function is called, the appropriate drawing steps will be performed.   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/15/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void GScreenClass::Flag_To_Redraw(bool complete)
{
    IsToUpdate = true;
    if (complete) {
        IsToRedraw = true;
    }
}

/***********************************************************************************************
 * GScreenClass::Input -- Fetches input and processes gadgets.                                 *
 *                                                                                             *
 *    This routine will fetch the keyboard/mouse input and dispatch this through the gadget    *
 *    system.                                                                                  *
 *                                                                                             *
 * INPUT:   key      -- Reference to the key code (for future examination).                    *
 *                                                                                             *
 *          x,y      -- Reference to mouse coordinates (for future examination).               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void GScreenClass::Input(KeyNumType& key, int& x, int& y)
{
    key = Keyboard->Check();

    x = Get_Mouse_X();
    y = Get_Mouse_Y();

    if (Buttons) {

        /*
        ** If any buttons need redrawing, they will do so in the Input routine, and
        ** they should draw themselves to the HidPage.  So, flag ourselves for a Blit
        ** to show the newly drawn buttons.
        */
        if (Buttons->Is_List_To_Redraw()) {
            Flag_To_Redraw(false);
        }

        GraphicViewPortClass* oldpage = Set_Logic_Page(HidPage);

        key = Buttons->Input();

        Set_Logic_Page(oldpage);

    } else {
        if (key) {
            key = Keyboard->Get();
        }
    }
    AI(key, x, y);
}

/***********************************************************************************************
 * GScreenClass::Add_A_Button -- Add a gadget to the game input system.                        *
 *                                                                                             *
 *    This will add a gadget to the game input system. The gadget will be processed in         *
 *    subsiquent calls to the GScreenClass::Input() function.                                  *
 *                                                                                             *
 * INPUT:   gadget   -- Reference to the gadget that will be added to the input system.        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void GScreenClass::Add_A_Button(GadgetClass& gadget)
{
    /*------------------------------------------------------------------------
    If this gadget is already in the list, remove it before adding it in:
    - If 1st gadget in list, use Remove_A_Button to remove it, to reset the
      value of 'Buttons' appropriately
    - Otherwise, just call the Remove function for that gadget to remove it
      from any list it may be in
    ------------------------------------------------------------------------*/
    if (Buttons == &gadget) {
        Remove_A_Button(gadget);
    } else {
        gadget.Remove();
    }

    /*------------------------------------------------------------------------
    Now add the gadget to our list:
    - If there are not buttons, start the list with this one
    - Otherwise, add it to the tail of the existing list
    ------------------------------------------------------------------------*/
    if (Buttons) {
        gadget.Add_Tail(*Buttons);
    } else {
        Buttons = &gadget;
    }
}

/***********************************************************************************************
 * GScreenClass::Remove_A_Button -- Removes a gadget from the game input system.               *
 *                                                                                             *
 * INPUT:   gadget   -- Reference to the gadget that will be removed from the input system.    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   'gadget' MUST be already a part of 'Buttons', or the new value of 'Buttons'     *
 *               will be invalid!                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void GScreenClass::Remove_A_Button(GadgetClass& gadget)
{
    Buttons = gadget.Remove();
}

/***********************************************************************************************
 * GScreenClass::Render -- General drawing dispatcher an display update function.              *
 *                                                                                             *
 *    This routine should be called in the main game loop (once every game frame). It will     *
 *    call the Draw_It() function if necessary. All rendering is performed to the LogicPage    *
 *    which is set to the HIDPAGE. After rendering has been performed, the HIDPAGE is          *
 *    copied to the visible page.                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This actually updates the graphic display. As a result it can take quite a      *
 *             while to perform.                                                               *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/15/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void GScreenClass::Render(void)
{
    // if (Buttons && Buttons->Is_List_To_Redraw()) {
    //	IsToRedraw = true;
    //}

    if (Rally_Point_Visible()) {
        // TODO: inefficient, would be better to replicate the rubber band logic to refresh only the cells below rally point line
        Map.Flag_To_Redraw(true);
    }

    if (IsToUpdate || IsToRedraw) {

        // WWMouse->Erase_Mouse(&HidPage, TRUE);
        GraphicViewPortClass* oldpage = Set_Logic_Page(HidPage);
        Draw_It(IsToRedraw);

        if (Rally_Point_Visible()) {
            Render_Rally_Point_Line();
        }

        if (Buttons)
            Buttons->Draw_All(false);

#ifdef SCENARIO_EDITOR
        /*
        ** Draw the Editor's buttons
        */
        if (Debug_Map) {
            if (Buttons) {
                // dynamically determine the bounds for a background box behind popup buttons
                const auto map_midpoint_y = (SeenBuff.Get_Height() - Map.Get_Tab_Height()) / 2;
                auto start_x = SeenBuff.Get_Width();
                auto start_y = SeenBuff.Get_Height();
                auto end_x = 0;
                auto end_y = 0;

                auto control = Buttons;
                auto popup_buttons_present = false;

                while (control != nullptr) {
                    // only match popup buttons (on the bottom of the screen)
                    if (control->Y >= map_midpoint_y) {
                        start_x = min(start_x, control->X);
                        start_y = min(start_y, control->Y);
                        end_x = max(end_x, control->X + control->Width);
                        end_y = max(end_y, control->Y + control->Height);

                        popup_buttons_present = true;
                    }

                    control = control->Get_Next();
                }

                if (popup_buttons_present) {
                    // draw a dialog background for the popup buttons
                    static auto constexpr dialog_margin = 10;
                    auto width = min(end_x + dialog_margin, SeenBuff.Get_Width() - dialog_margin) - (start_x - dialog_margin);
                    auto height = min(end_y + dialog_margin, SeenBuff.Get_Height() - dialog_margin) - (start_y - dialog_margin);

                    // add extra padding on the top and right to fit label text
                    start_x -= dialog_margin;
                    start_y -= (dialog_margin * 4);
                    width += dialog_margin;
                    height += (dialog_margin * 4);

                    Dialog_Box(start_x, start_y, width, height);

                    // print the selected object name
                    if (CurrentObject.Count() > 0) {
                        // init font
                        Fancy_Text_Print(TXT_NONE, 0, 0, CC_GREEN, TBLACK, TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
                        Draw_Caption(
                            Text_String(CurrentObject[0]->Full_Name()),
                            OPTION_DIALOG,
                            start_x,
                            start_y,
                            width
                        );
                    }
                }

                Buttons->Draw_All();
            }
        }
#endif
        /*
        ** Draw the multiplayer message system to the Hidpage at this point.
        ** This way, they'll Blit along with the rest of the map.
        */
        if (Messages.Num_Messages() > 0) {
            Messages.Set_Width(Lepton_To_Cell(Map.TacLeptonWidth) * ICON_PIXEL_W);
        }
        Messages.Draw();

#ifndef REMASTER_BUILD
        Blit_Display();
#endif
        IsToUpdate = false;
        IsToRedraw = false;

        Set_Logic_Page(oldpage);
    }
}

#ifdef CHEAT_KEYS

#define MAX_SCREENS_SAVED 30 * 15 // Enough for 30 seconds @ 15 fps

GraphicBufferClass* ScreenList[MAX_SCREENS_SAVED];
int CurrentScreen = 0;
bool ScreenRecording = false;

void Add_Current_Screen(void)
{
#if (0) // ST - 1/2/2019 5:51PM
    if (ScreenRecording) {
        ScreenList[CurrentScreen] = new GraphicBufferClass;
        ScreenList[CurrentScreen]->Init(SeenBuff.Get_Width(), SeenBuff.Get_Height(), NULL, 0, (GBC_Enum)0);
        SeenBuff.Blit(*ScreenList[CurrentScreen]);

        CurrentScreen++;

        if (CurrentScreen == MAX_SCREENS_SAVED) {

            char filename[20];
            for (int i = 0; i < MAX_SCREENS_SAVED; i++) {
                sprintf(filename, "SCRN%04d.PCX", i);
                Write_PCX_File(filename, *ScreenList[i], (unsigned char*)CurrentPalette);
                delete ScreenList[i];
            }

            CurrentScreen = 0;
            ScreenRecording = 0;
        }
    }
#endif
}

#endif // CHEAT_KEYS

extern bool CanVblankSync;

/***********************************************************************************************
 * GScreenClass::Blit_Display -- Redraw the display from the hidpage to the seenpage.          *
 *                                                                                             *
 *    This routine is used to copy the correct display from the HIDPAGE                        *
 *    to the SEENPAGE.                                                                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1994 JLB : Created.                                                                 *
 *   05/01/1994 JLB : Converted to member function.                                            *
 *=============================================================================================*/
void GScreenClass::Blit_Display(void)
{
#if (0)
    if (HidPage.Get_IsDirectDraw() && (Options.GameSpeed > 1 || Options.ScrollRate == 6 && CanVblankSync)) {
        WWMouse->Draw_Mouse(&HidPage);
        SeenBuff.Get_Graphic_Buffer()->Get_DD_Surface()->Flip(NULL, DDFLIP_WAIT);
        SeenBuff.Blit(HidPage, 0, 0, 0, 0, SeenBuff.Get_Width(), SeenBuff.Get_Height(), false);
#ifdef CHEAT_KEYS
        Add_Current_Screen();
#endif
        // HidPage.Blit ( SeenBuff , 0 , 0 , 0 , 0 , HidPage.Get_Width() , HidPage.Get_Height() , (BOOL) FALSE );
        WWMouse->Erase_Mouse(&HidPage, false);
    } else {
#else //(0)
    WWMouse->Draw_Mouse(&HidPage);
    HidPage.Blit(SeenBuff, 0, 0, 0, 0, HidPage.Get_Width(), HidPage.Get_Height(), false);
#ifdef CHEAT_KEYS
    Add_Current_Screen();
#endif
    WWMouse->Erase_Mouse(&HidPage, false);
#endif //(0)
#if (0)
    }
#endif //(0)
}

bool GScreenClass::Rally_Point_Visible() const
{
    return Map.VisibleRallyPointSource != nullptr
        && Map.VisibleRallyPointSource->Can_Have_Rally_Point()
        && Target_Legal(Map.VisibleRallyPointSource->RallyPoint);
}

/**
 * Given a path between two co-ords (where the end point is a co-ord that is not currently
 * visible to the player) determine the last visible pixel of the path (at the screen edge).
 *
 * Used to partially draw a line which is cut off by the edge of these screen.
 */
static bool Get_Screen_Edge_Pixel_For_Path(
    const int& path_start_coord_x,
    const int& path_start_coord_y,
    const int& path_end_coord_x,
    const int& path_end_coord_y,
    const int& path_start_pixel_x,
    const int& path_start_pixel_y,
    int& path_end_pixel_x,
    int& path_end_pixel_y
)
{
    // calculate the line from lhs point to the edge of the screen, in the direction of the rhs point
    auto dirX = static_cast<double>(path_end_coord_x) - path_start_coord_x;
    auto dirY = static_cast<double>(path_end_coord_y) - path_start_coord_y;

    const auto length = std::sqrt(dirX * dirX + dirY * dirY);

    if (length < 1) {
        return false;
    }

    dirX /= length;
    dirY /= length;

    const auto rightEdge = static_cast<double>(Map.IsSidebarActive ? Map.SideX - 1 : SeenBuff.Get_Width() - 1);
    const auto bottomEdge = static_cast<double>(SeenBuff.Get_Height()) - 1;
    const auto topEdge = static_cast<double>(Map.Get_Tab_Height());

    // Calculate the distance to the closest screen edge to clip the line
    const auto tToXEdge = (dirX > 0)
        ? (rightEdge - path_start_pixel_x) / dirX
        : (dirX < 0) ? (0 - path_start_pixel_x) / dirX : 1e18;
    const auto tToYEdge = (dirY > 0)
        ? (bottomEdge - path_start_pixel_y) / dirY
        : (dirY < 0) ? (topEdge - path_start_pixel_y) / dirY : 1e18;

    const auto tToEdge = std::min(tToXEdge, tToYEdge);

    path_end_pixel_x = static_cast<int>(path_start_pixel_x + tToEdge * dirX);
    path_end_pixel_y = static_cast<int>(path_start_pixel_y + tToEdge * dirY);

    return true;
}

void GScreenClass::Render_Rally_Point_Line() const
{
    if (!LogicPage->Lock()) {
        return;
    }

    // unpack rally start and end point co-ords
    int coords[4] {
        Coord_X(VisibleRallyPointSource->Center_Coord()),
        Coord_Y(VisibleRallyPointSource->Center_Coord()),
        Coord_X(As_Coord(VisibleRallyPointSource->RallyPoint)),
        Coord_Y(As_Coord(VisibleRallyPointSource->RallyPoint))
    };
    const auto& [ start_x, start_y, end_x, end_y ] = coords;

    // attempt to convert co-ords to pixel values
    int start_pixel_x, start_pixel_y, end_pixel_x, end_pixel_y;
    const auto start_ok = Map.Coord_To_Pixel(VisibleRallyPointSource->Center_Coord(), start_pixel_x, start_pixel_y);
    const auto end_ok = Map.Coord_To_Pixel(As_Coord(VisibleRallyPointSource->RallyPoint), end_pixel_x, end_pixel_y);

    if (!start_ok && ! end_ok) {
        // neither rally point is in a viewable section of the map, abort
        LogicPage->Unlock();
        return;
    }

    // one of the rally lines points is out of view
    if (start_ok && !end_ok) {
        // calculate the last visible pixel of the line from start point to the end point
        Get_Screen_Edge_Pixel_For_Path(
            start_x,
            start_y,
            end_x,
            end_y,
            start_pixel_x,
            start_pixel_y,
            end_pixel_x,
            end_pixel_y
        );
    } else if (end_ok && !start_ok) {
        // calculate the last visible pixel of the line from end point to the start point
        Get_Screen_Edge_Pixel_For_Path(
            end_x,
            end_y,
            start_x,
            start_y,
            end_pixel_x,
            end_pixel_y,
            start_pixel_x,
            start_pixel_y
        );
    }

    // BUG: checks prevent line drawing over sidebar or tabs but can cause pitch/angle of the line to be slightly off
    if (Map.IsSidebarActive && start_pixel_x >= Map.SideX) {
        start_pixel_x = Map.SideX - 1;
    }
    if (start_pixel_y < Map.Get_Tab_Height()) {
        start_pixel_y = Map.Get_Tab_Height() + 1;
    }

    if (Map.IsSidebarActive && end_pixel_x >= Map.SideX) {
        end_pixel_x = Map.SideX - 1;
    }
    if (end_pixel_y < Map.Get_Tab_Height()) {
        end_pixel_y = Map.Get_Tab_Height() + 1;
    }

    LogicPage->Draw_Line(start_pixel_x, start_pixel_y, end_pixel_x, end_pixel_y, LTGREEN);
    LogicPage->Unlock();
}

TO_JSON(GScreenClass)
{
    BITFIELD_TO_JSON(IsToRedraw);
    BITFIELD_TO_JSON(IsToUpdate);
    OBJECT_TARGET_PTR_TO_JSON(VisibleRallyPointSource);
}

FROM_JSON(GScreenClass)
{
    BITFIELD_FROM_JSON(IsToRedraw);
    BITFIELD_FROM_JSON(IsToUpdate);
    TARGET_CONST_PTR_FROM_JSON_WITH_TYPE(VisibleRallyPointSource, BuildingClass);
}
