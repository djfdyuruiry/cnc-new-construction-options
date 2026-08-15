#include <ranges>

#include "function.h"
#include "mapeditsidebar.h"
#include "tiberiandawnsettings.h"

typedef enum
{
    MINIMAP = MAP_AREA + 1, // extend scenario editor button IDs
    OVERLAY_BUTTON,
    TERRAIN_BUTTON,
    UNITS_BUTTON,
    BUILDINGS_BUTTON,
    WAYPOINTS_BUTTON,
    TRIGGERS_BUTTON,
    OVERLAY_GRID,
    TERRAIN_OBJECT_LIST,
    UNITS_GRID,
    BUILDING_OBJECT_LIST,
    WAYPOINTS_LIST,
    TRIGGERS_LIST,
    PREVIOUS_BUTTON,
    NEXT_BUTTON,
    GOTO_WAYPT_BUTTON,
    CLEAR_WAYPT_BUTTON,
    ADD_TRIGGER_BUTTON,
    EDIT_TRIGGER_BUTTON,
    DELETE_TRIGGER_BUTTON
} SidebarControls;

void MapEditorSidebar::Init_Controls()
{
    // virtual control for minimap to support input events
    Controls[MINIMAP] = std::make_unique<ControlClass>(
        MINIMAP,
        Dimensions[MINIMAP].X,
        Dimensions[MINIMAP].Y,
        Dimensions[MINIMAP].W,
        Dimensions[MINIMAP].H,
        GadgetClass::LEFTRELEASE,
        false
    );

    // selection buttons
    static const auto button_text_flags = TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW;

    static const std::map<SidebarControls, const char*> buttons = {
        {OVERLAY_BUTTON, "Overlay"},
        {TERRAIN_BUTTON, "Terrain"},
        {UNITS_BUTTON, "Units"},
        {BUILDINGS_BUTTON, "Buildings"},
        {WAYPOINTS_BUTTON, "Waypoints"},
        {TRIGGERS_BUTTON, SeenBuff.Get_Width() <= GBUFF_INIT_WIDTH ? "Triggers" : "Cell Triggers"},
        {PREVIOUS_BUTTON, "Previous"},
        {NEXT_BUTTON, "Next"},
        {GOTO_WAYPT_BUTTON, "Go To"},
        {CLEAR_WAYPT_BUTTON, "Clear"},
        {ADD_TRIGGER_BUTTON, "Add"},
        {EDIT_TRIGGER_BUTTON, "Edit"},
        {DELETE_TRIGGER_BUTTON, "Delete"}
    };

    for (const auto& [id, text] : buttons) {
        Controls[id] = std::make_unique<TextButtonClass>(
            id,
            text,
            button_text_flags,
            Dimensions[id].X,
            Dimensions[id].Y,
            Dimensions[id].W,
            Dimensions[id].H
        );

        if (id > TRIGGERS_BUTTON) {
            Controls[id].get()->Disable(true);
        }
    }

    // virtual controls for grids and lists to support input events
    for (auto i = OVERLAY_GRID; i <= BUILDING_OBJECT_LIST; ++i) {
        Controls[i] = std::make_unique<ControlClass>(
            i,
            Dimensions[i].X,
            Dimensions[i].Y,
            Dimensions[i].W,
            Dimensions[i].H,
            GadgetClass::LEFTRELEASE,
            false
        );
        Controls[i]->Disable();
    }

    // list controls
    static const auto up_btn = Hires_Retrieve("BTN-UP.SHP");
    static const auto down_btn = Hires_Retrieve("BTN-DN.SHP");

    for (const auto& id : { WAYPOINTS_LIST, TRIGGERS_LIST }) {
        Controls[id] = std::make_unique<ListClass>(
            id,
            Dimensions[id].X,
            Dimensions[id].Y,
            Dimensions[id].W,
            Dimensions[id].H,
            TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
            up_btn,
            down_btn
        );
        Controls[id]->Disable(true);
    }

    // waypoint list (text labels, tracker list and populate UI list)

    if (WaypointText.empty()) {
        // initialise static waypoint text strings
        for (int i = 0; i < WAYPT_HOME; i++) {
            auto text_ptr = std::make_unique<char[]>(3);
            sprintf(text_ptr.get(), "%d", i);

            WaypointText.emplace_back(std::move(text_ptr));
        }
    }

    if (WaypointLookup.empty()) {
        // initialise static tracker list for mapping selected index to waypoint
        WaypointLookup.push_back(WAYPT_HOME);
        WaypointLookup.push_back(WAYPT_REINF);

        for (auto i = static_cast<WaypointType>(0); i < WAYPT_HOME; ++i) {
            WaypointLookup.push_back(i);
        }
    }

    auto& waypoint_list = Get_Control<WAYPOINTS_LIST, ListClass>();

    while (waypoint_list.Count() > 0) {
        waypoint_list.Remove_Item(0);
    }

    // add const char strings
    waypoint_list.Add_Item("HOME");
    waypoint_list.Add_Item("REINF");

    // add dynamic char strings
    for (const auto& text_ptr : WaypointText) {
        waypoint_list.Add_Item(text_ptr.get());
    }
}

void MapEditorSidebar::Init_Dimensions()
{
    /**
     * Minimap
     */
#ifdef MEGAMAPS
    Dimensions[MINIMAP] = {
        .X = X + ((W - 130) / 2),
        .Y = Y + ControlMargin,
        .W = 130,
        .H = 130
    };
#else
    Dimensions[MINIMAP] = {
        X + ((W - 64) / 2),
        Y + ControlMargin,
        64,
        64
    };
#endif

    /**
     * Map object selection buttons
     */
    const auto button_width = (W - ControlMargin * 3) / 2;

    // object select first row
    Dimensions[OVERLAY_BUTTON] = {
        .X = X + ControlMargin,
        .Y = (Dimensions[MINIMAP].Y + Dimensions[MINIMAP].H) + (ControlMargin * 2),
        .W = button_width,
        .H = ButtonHeight
    };

    Dimensions[TERRAIN_BUTTON] = Dimensions[OVERLAY_BUTTON];
    Dimensions[TERRAIN_BUTTON].X += button_width + ControlMargin; // adjust right

    // object select second row
    Dimensions[UNITS_BUTTON] = Dimensions[OVERLAY_BUTTON];
    Dimensions[UNITS_BUTTON].Y += ButtonHeight + ControlMargin;  // adjust down

    Dimensions[BUILDINGS_BUTTON] = Dimensions[TERRAIN_BUTTON];
    Dimensions[BUILDINGS_BUTTON].Y += ButtonHeight + ControlMargin;  // adjust down

    // object select third row
    Dimensions[WAYPOINTS_BUTTON] = Dimensions[UNITS_BUTTON];
    Dimensions[WAYPOINTS_BUTTON].Y += ButtonHeight + ControlMargin;  // adjust down

    Dimensions[TRIGGERS_BUTTON] = Dimensions[BUILDINGS_BUTTON];
    Dimensions[TRIGGERS_BUTTON].Y += ButtonHeight + ControlMargin;  // adjust down

    /**
     * Sidebar content panels
     */
    for (auto i = OVERLAY_GRID; i <= TRIGGERS_LIST; ++i) {
        Dimensions[i] = Dimensions[WAYPOINTS_BUTTON];
        Dimensions[i].Y += Dimensions[WAYPOINTS_BUTTON].H + ControlMargin;
        Dimensions[i].W = W - (ControlMargin * 2);
        Dimensions[i].H = H - (Dimensions[i].Y - Y) - FooterHeight;
    }

    // grid and list navigation
    Dimensions[PREVIOUS_BUTTON] = {
        .X = X + ControlMargin,
        .Y = (Y + H - FooterHeight) + ControlMargin,
        .W = button_width,
        .H = ButtonHeight
    };

    Dimensions[NEXT_BUTTON] = Dimensions[PREVIOUS_BUTTON];
    Dimensions[NEXT_BUTTON].X += ControlMargin + button_width;

    // waypoint buttons
    Dimensions[GOTO_WAYPT_BUTTON] = Dimensions[PREVIOUS_BUTTON];
    Dimensions[CLEAR_WAYPT_BUTTON] = Dimensions[NEXT_BUTTON];

    // trigger buttons
    const auto footer_width = W - (ControlMargin - 2);
    Dimensions[GOTO_WAYPT_BUTTON] = Dimensions[PREVIOUS_BUTTON];
    const auto trigger_button_width = ((footer_width - (ControlMargin * 2)) / 3);

    Dimensions[ADD_TRIGGER_BUTTON] = Dimensions[PREVIOUS_BUTTON];
    Dimensions[ADD_TRIGGER_BUTTON].W = trigger_button_width;

    Dimensions[EDIT_TRIGGER_BUTTON] = Dimensions[ADD_TRIGGER_BUTTON];
    Dimensions[EDIT_TRIGGER_BUTTON].X += Dimensions[ADD_TRIGGER_BUTTON].W + ControlMargin;

    Dimensions[DELETE_TRIGGER_BUTTON] = Dimensions[EDIT_TRIGGER_BUTTON];
    Dimensions[DELETE_TRIGGER_BUTTON].X += Dimensions[EDIT_TRIGGER_BUTTON].W  + ControlMargin;
    Dimensions[DELETE_TRIGGER_BUTTON].W -= ControlMargin;
}

void MapEditorSidebar::Init(MapEditClass* parent)
{
    if (parent == nullptr) {
        return;
    }

    // init state
    Parent = parent;
    Controls.clear();

    // reset pagination
    OverlayGridPager = {};
    TerrainPager = {};
    UnitsGridPager = {};
    BuildingListPager = {};

    // reset current object context
    CurrentObject = nullptr;
    HelpText.reset();
    HelpTextX = 0;
    HelpTextY = 0;

    Init_Dimensions();
    Init_Controls();
}

void MapEditorSidebar::Add_This()
{
    if (Parent == nullptr) {
        return;
    }

    for (auto i = MINIMAP; i <= DELETE_TRIGGER_BUTTON; ++i) {
        Parent->Add_A_Button(*Controls[i]);
    }

    if (
        !Get_Control<OVERLAY_GRID, ControlClass>().IsEnabled()
        && !Get_Control<TERRAIN_OBJECT_LIST, ControlClass>().IsEnabled()
        && !Get_Control<UNITS_GRID, ControlClass>().IsEnabled()
        && !Get_Control<BUILDING_OBJECT_LIST, ControlClass>().IsEnabled()
        && !Get_Control<WAYPOINTS_LIST, ListClass>().IsEnabled()
        && !Get_Control<TRIGGERS_LIST, ControlClass>().IsEnabled()
    ) {
        // nothing selected, so show first panel
        constexpr auto fake_input = static_cast<KeyNumType>(OVERLAY_BUTTON | KN_BUTTON);
        On_Input(fake_input, true);
    }
}

void MapEditorSidebar::Remove_This()
{
    if (Parent == nullptr) {
        return;
    }

    for (auto i = MINIMAP; i <= TRIGGERS_LIST; ++i) {
        Parent->Remove_A_Button(*Controls[i]);
    }
}

/**
 * Add info for type instances that have valid image data to the given catalog.
 *
 * @tparam T Enum type that holds instances (UnitType etc.)
 * @tparam U ObjectTypeClass subclass
 * @param catalog Vector to store type instance pointers in
 */
template <SupportedByTdTypeConverter T, typename U>
static void Populate_Object_Catalog(std::vector<MapEditorSidebar::ObjectCatalogItem>& catalog)
{
    for (const auto& instance : TdTypeConverter::Get_Valid_Instances<T>()) {
        if constexpr (std::is_same_v<T, TemplateType>) {
            if (instance >= TEMPLATE_NONE) {
                // ordering is different to other types
                continue;
            }
        } else if constexpr (std::is_same_v<T, OverlayType>) {
            if (instance < 0 || instance > OVERLAY_TIBERIUM1 && instance <= OVERLAY_TIBERIUM12) {
                // we only want the first tiberium instance
                continue;
            }
        } else {
            if (instance < 0) {
                continue;
            }
        }

        const auto& instance_obj = U::As_Reference(instance);

        if (instance_obj.Get_Image_Data() == nullptr) {
            continue;
        }

        if constexpr (std::is_same_v<T, StructType>) {
            if (reinterpret_cast<const BuildingTypeClass*>(&instance_obj)->IsWall) {
                // ignore wall buildings, these are shown in the overlay grid instead
                continue;
            }
        }

        MapEditorSidebar::ObjectCatalogItem entry;
        entry.ObjectType = &instance_obj;

        catalog.emplace_back(entry);
    }
}

void MapEditorSidebar::Purge_Theater_Objects()
{
    /**
     * Purge catalogs for object types that are theater dependant (object count may change).
     */
    OverlayCatalog.clear();
    TerrainCatalog.clear();
    BuildingsCatalog.clear();
}

void MapEditorSidebar::Refresh_Trigger_List()
{
    /**
     * Refresh cell trigger info.
     */
    auto& trigger_list = Get_Control<TRIGGERS_LIST, ListClass>();

    // clear down trigger list and UI
    CellTriggerList.clear();

    while (trigger_list.Count() > 0) {
        trigger_list.Remove_Item(0);
    }

    // populate with current cell triggers
    for (auto i = 0; i < Triggers.Count(); i++) {
        auto trigger = Triggers.Ptr(i);

        if (trigger == nullptr || (trigger->Event != EVENT_PLAYER_ENTERED && trigger->Event != EVENT_CELLFIRST)) {
            // filter for cell triggers
            continue;
        }

        CellTriggerList.emplace_back(trigger);
        trigger_list.Add_Item(trigger->Get_Name());
    }
}

void MapEditorSidebar::Refresh_For_Scenario()
{
    /**
     * Reset pagination for object types that are theater dependant (page count may change).
     */
    OverlayGridPager = {};
    TerrainPager = {};
    BuildingListPager = {};

    Refresh_Trigger_List();
}

/**
 * Dumps out object graphics/icons to fill the object area, using start_idx and end_idx for pagination.
 */
void MapEditorSidebar::Render_Object_List(
    const int control, std::vector<ObjectCatalogItem>& objects, ObjectListPager& pager
)
{
    // TODO: Consider alternatives, maybe draw stamp with clear template - very hard to see smudges
    /**
     * Draw background and border
     */
    InGameFillTexture.Draw_Rectangle(
        *LogicPage,
        Dimensions[control].X,
        Dimensions[control].Y,
        Dimensions[control].W,
        Dimensions[control].H
    );

    LogicPage->Draw_Rect(
        Dimensions[control].X,
        Dimensions[control].Y,
        Dimensions[control].X + Dimensions[control].W,
        Dimensions[control].Y + Dimensions[control].H,
        GRAY
    );

    if (pager.CurrentIndex == -1) {
        pager.CurrentIndex = 0;
        pager.CurrentDepth = 0;
        pager.PageStartIndexes.emplace_back(0);
    }

    // reset dimensions for not rendered items
    for (auto i = 0; i < pager.CurrentIndex; ++i) {
        objects[i].Dimensions.Reset();
    }

    /**
     * Draw objects
     */
    auto x = ControlMargin;
    auto y = ControlMargin;
    auto idx = pager.CurrentIndex;

    // clipping window for list
    WindowList[WINDOW_EDITOR][WINDOWX] = Dimensions[control].X;
    WindowList[WINDOW_EDITOR][WINDOWY] = Dimensions[control].Y;
    WindowList[WINDOW_EDITOR][WINDOWWIDTH] = Dimensions[control].W;
    WindowList[WINDOW_EDITOR][WINDOWHEIGHT] = Dimensions[control].H;

    // walk down the list height
    while (y < Dimensions[control].H && idx < objects.size()) {
        auto next_y = y; // track the deepest y value of a rendered object

        // walk across the list width
        while (x < Dimensions[control].W && idx < objects.size()) {
            int end_x = x;
            int end_y = y;

            // render the object, with end vars to determine width
            objects[idx].ObjectType->Display(x, y, WINDOW_EDITOR, PlayerPtr->Class->House, end_x, end_y);

            if (end_x <= Dimensions[control].W && end_y <= Dimensions[control].H) {
                // OK - object fits
                auto& catalog_dimensions = objects[idx].Dimensions;

                catalog_dimensions.X = Dimensions[control].X + x;
                catalog_dimensions.Y = Dimensions[control].Y + y;
                catalog_dimensions.W = end_x - x;
                catalog_dimensions.H = end_y - y;

                idx++;

                x = end_x + ControlMargin;
                next_y = max(next_y, end_y);
            } else {
                // KO - object too big, erase partial drawing and go to next y line
                InGameFillTexture.Draw_Rectangle(
                    *LogicPage,
                    Bound(Dimensions[control].X + x, Dimensions[control].X, Dimensions[control].X + Dimensions[control].W),
                    Bound(Dimensions[control].Y + y, Dimensions[control].Y, Dimensions[control].Y + Dimensions[control].H),
                    Bound(end_x, 0, Dimensions[control].W - x),
                    Bound(end_y, 0, Dimensions[control].H - y)
                );
                break;
            }
        }

        x = ControlMargin * 2;
        y = next_y + ControlMargin * 2;
    }

    const auto has_next_page = idx < objects.size();

    if (pager.PageStartIndexes.size() <= pager.CurrentDepth + 1 && has_next_page) {
        pager.PageStartIndexes.emplace_back(idx);
    }

    // reset dimensions for not rendered items
    for (auto i = idx; i < objects.size(); ++i) {
        objects[i].Dimensions.Reset();
    }

    // disable pagination buttons if only one page available
    if (pager.CurrentIndex == 0 && !has_next_page) {
        if (Controls[PREVIOUS_BUTTON]->IsEnabled()) {
            Controls[PREVIOUS_BUTTON]->Disable();
        }

        if (Controls[NEXT_BUTTON]->IsEnabled()) {
            Controls[NEXT_BUTTON]->Disable();
        }

        return;
    }

    // previous button toggle
    if (pager.CurrentIndex != 0 && !Controls[PREVIOUS_BUTTON]->IsEnabled()) {
        Controls[PREVIOUS_BUTTON]->Enable();
    } else if (pager.CurrentIndex == 0 && Controls[PREVIOUS_BUTTON]->IsEnabled()) {
        Controls[PREVIOUS_BUTTON]->Disable();
    }

    // next button toggle
    if (has_next_page && !Controls[NEXT_BUTTON]->IsEnabled()) {
        Controls[NEXT_BUTTON]->Enable();
    } else if (!has_next_page && Controls[NEXT_BUTTON]->IsEnabled()) {
        Controls[NEXT_BUTTON]->Disable();
    }
}

/**
 * Draws a 'page' worth of object graphics/icons in a table of uniform size cells. page_number is used
 * for pagination.
 */
void MapEditorSidebar::Render_Object_Grid(const int control, std::vector<ObjectCatalogItem>& objects, GridPager& pager)
{
    constexpr auto cell_width = 64;
    constexpr auto cell_height = 48;

    const int row_count = floor(static_cast<float>(Dimensions[control].H) / cell_height);
    const int cell_count = floor(static_cast<float>(Dimensions[control].W) / cell_width);

    if (pager.CurrentPage == -1) {
        pager.ItemsPerPage = row_count * cell_count;
        pager.PageCount = ceil(static_cast<float>(objects.size()) / pager.ItemsPerPage);
        pager.CurrentPage = 0;
    }

    const auto x_offset = (Dimensions[control].W - (cell_width * cell_count)) / 2;
    const auto y_offset = (Dimensions[control].H - (cell_height * row_count)) / 2;

    auto idx = min(objects.size(), pager.CurrentPage * pager.ItemsPerPage);
    const auto end_idx = min(objects.size(), idx + pager.ItemsPerPage);

    // reset dimensions for not rendered items
    for (auto i = 0; i < idx; ++i) {
        objects[i].Dimensions.Reset();
    }

    for (auto row = 0 ; row < row_count; row++)
    {
        auto at_end = false;

        for (auto cell = 0 ; cell < cell_count; cell++)
        {
            if (idx >= end_idx) {
                at_end = true;
                break;
            }

            objects[idx].Dimensions.X = Dimensions[control].X + x_offset + (cell * cell_width);
            objects[idx].Dimensions.Y = Dimensions[control].Y + y_offset + (row * cell_height);
            objects[idx].Dimensions.W = cell_width;
            objects[idx].Dimensions.H = cell_height;

            Parent->Draw_Member(
                objects[idx].ObjectType,
                0,
                0,
                PlayerPtr->Class->House,
                objects[idx].Dimensions.X,
                objects[idx].Dimensions.Y
            );
            idx++;
        }

        if (at_end) {
            break;
        }
    }

    // reset dimensions for not rendered items
    for (auto i = idx; i < objects.size(); ++i) {
        objects[i].Dimensions.Reset();
    }

    // disable pagination buttons if only one page available
    if (pager.PageCount == 1) {
        if (Controls[PREVIOUS_BUTTON]->IsEnabled()) {
            Controls[PREVIOUS_BUTTON]->Disable();
        }

        if (Controls[NEXT_BUTTON]->IsEnabled()) {
            Controls[NEXT_BUTTON]->Disable();
        }

        return;
    }

    // previous button toggle
    if (pager.CurrentPage > 0 && !Controls[PREVIOUS_BUTTON]->IsEnabled()) {
        Controls[PREVIOUS_BUTTON]->Enable();
    } else if (pager.CurrentPage == 0 && Controls[PREVIOUS_BUTTON]->IsEnabled()) {
        Controls[PREVIOUS_BUTTON]->Disable();
    }

    // next button toggle
    if (pager.CurrentPage < pager.PageCount - 1 && !Controls[NEXT_BUTTON]->IsEnabled()) {
        Controls[NEXT_BUTTON]->Enable();
    } else if (pager.CurrentPage >= pager.PageCount - 1 && Controls[NEXT_BUTTON]->IsEnabled()) {
        Controls[NEXT_BUTTON]->Disable();
    }
}

static int Get_Menu_Color_For_Cell(const CELL raw_cell, const CellClass& cell)
{
    if (cell.IsWaypoint && ScenPlayer == SCEN_PLAYER_MPLAYER) {
        for (auto i = 0; i < MPlayerMax; i++) {
            if (raw_cell == Scen.Waypoint[i]) {
                // multiplayer start location
                return 127;
            }
        }
    }

    auto occupier = cell.Cell_Occupier();

    if (occupier != nullptr) {
        // pick a color based on occupier type
        switch (occupier->What_Am_I()) {
            case RTTI_TEMPLATE: {
                return PINK;
            }

            case RTTI_TERRAIN: {
                const auto terrain = reinterpret_cast<TerrainClass*>(occupier);
                // trees or rocks
                return terrain->Full_Name() == TXT_TREE ? DKGREEN : DKGRAY;
            }

            case RTTI_AIRCRAFT:
            case RTTI_INFANTRY:
            case RTTI_UNIT:
            case RTTI_BUILDING: {
                return HouseClass::As_Pointer(cell.Cell_Occupier()->Owner())->Class->Color;
            }

            default: break;
        }
    }

    // pick a color based on land type
    if (cell.TType >= TEMPLATE_ROAD1 && cell.TType <= TEMPLATE_ROAD43) {
        // road
        return CC_TAN;
    }

    if (cell.TType >= TEMPLATE_BRIDGE1 && cell.TType <= TEMPLATE_BRIDGE4D) {
        // bridge
        return CC_TAN;
    }

    return Ground[cell.Land_Type()].Color;
}

/**
 * TODO: Align colors better with skirmish minimap which looks easier to visually process
 */
void MapEditorSidebar::Render_Minimap()
{
    const auto minimap_bottom_right_x = Dimensions[MINIMAP].X + Dimensions[MINIMAP].W;
    const auto minimap_bottom_right_y = Dimensions[MINIMAP].Y + Dimensions[MINIMAP].H;

    // draw minimap background
    LogicPage->Fill_Rect(
        Dimensions[MINIMAP].X + 1,
        Dimensions[MINIMAP].Y + 1,
        minimap_bottom_right_x - 1,
        minimap_bottom_right_y - 1,
        Ground[LAND_CLEAR].Color
    );

    Hide_Mouse();
    LogicPage->Lock();

#ifdef MEGAMAPS
    const auto scale_map = Parent->IniMapCellX + Parent->IniMapCellWidth <= 64
        && Parent->IniMapCellY + Parent->IniMapCellHeight <= 64; // are the map bounds within normal map size?
#else
    const auto scale_map = true;
#endif

    Parent->Iterate_Over_Map_Cells([&](const auto raw_cell, const auto& cell) {
        const auto cell_x = Cell_X(raw_cell);
        const auto cell_y = Cell_Y(raw_cell);

        if (scale_map && (cell_x > 64 || cell_y > 64)) {
            // if scaling the map, only render original map cells
            return;
        }

        auto color = Get_Menu_Color_For_Cell(raw_cell, cell);

        if (scale_map) {
            for (int x = 0; x < 2; ++x) {
                for (int y = 0; y < 2; ++y) {
                    LogicPage->Put_Pixel(
                        Dimensions[MINIMAP].X + (cell_x * 2) + x + 1,
                        Dimensions[MINIMAP].Y + (cell_y * 2) + y + 1,
                        color
                    );
                }
            }
        } else {
            LogicPage->Put_Pixel(
                Dimensions[MINIMAP].X + cell_x + 1,
                Dimensions[MINIMAP].Y + cell_y + 1,
                color
            );
        }
    });

    // draw scenario bounds for map
    if (scale_map) {
        LogicPage->Draw_Rect(
            Dimensions[MINIMAP].X + (Parent->IniMapCellX * 2) + 1,
            Dimensions[MINIMAP].Y + (Parent->IniMapCellY * 2) + 1,
            Dimensions[MINIMAP].X + (Parent->IniMapCellX * 2 + Parent->IniMapCellWidth * 2) + 1,
            Dimensions[MINIMAP].Y + (Parent->IniMapCellY * 2 + Parent->IniMapCellHeight * 2) + 1,
            WHITE
        );
    } else {
        LogicPage->Draw_Rect(
            Dimensions[MINIMAP].X + Parent->IniMapCellX + 1,
            Dimensions[MINIMAP].Y + Parent->IniMapCellY + 1,
            Dimensions[MINIMAP].X + (Parent->IniMapCellX + Parent->IniMapCellWidth) + 1,
            Dimensions[MINIMAP].Y + (Parent->IniMapCellY + Parent->IniMapCellHeight) + 1,
            WHITE
        );
    }

    LogicPage->Unlock();

    // draw minimap border
    LogicPage->Draw_Rect(
        Dimensions[MINIMAP].X,
        Dimensions[MINIMAP].Y,
        minimap_bottom_right_x,
        minimap_bottom_right_y,
        GRAY
    );

    Show_Mouse();
}

void MapEditorSidebar::Render()
{
    if (Parent == nullptr) {
        return;
    }

    // background
    Dialog_Box(X, Y, W, H);

    // left margin
    LogicPage->Draw_Line(X - 1, Y, X - 1, Y + H, GRAY);

    Render_Minimap();

    // active content panel
    if (Get_Control<OVERLAY_GRID, ControlClass>().IsEnabled()) {
        if (OverlayCatalog.empty()) {
            Populate_Object_Catalog<OverlayType, OverlayTypeClass>(OverlayCatalog);

            OverlayGridPager.ItemCount = OverlayCatalog.size();
        }

        Render_Object_Grid(OVERLAY_GRID, OverlayCatalog, OverlayGridPager);
    } else if (Get_Control<TERRAIN_OBJECT_LIST, ControlClass>().IsEnabled()) {
        if (TerrainCatalog.empty()) {
            Populate_Object_Catalog<TemplateType, TemplateTypeClass>(TerrainCatalog);
            Populate_Object_Catalog<TerrainType, TerrainTypeClass>(TerrainCatalog);
            Populate_Object_Catalog<SmudgeType, SmudgeTypeClass>(TerrainCatalog);

            TerrainPager.ItemCount = TerrainCatalog.size();
        }

        Render_Object_List(TERRAIN_OBJECT_LIST, TerrainCatalog, TerrainPager);
    } else if (Get_Control<UNITS_GRID, ControlClass>().IsEnabled()) {
        if (UnitsCatalog.empty()) {
            Populate_Object_Catalog<InfantryType, InfantryTypeClass>(UnitsCatalog);
            Populate_Object_Catalog<UnitType, UnitTypeClass>(UnitsCatalog);
            Populate_Object_Catalog<AircraftType, AircraftTypeClass>(UnitsCatalog);

            UnitsGridPager.ItemCount = UnitsCatalog.size();
        }

        Render_Object_Grid(UNITS_GRID, UnitsCatalog, UnitsGridPager);
    } else if (Get_Control<BUILDING_OBJECT_LIST, ControlClass>().IsEnabled()) {
        if (BuildingsCatalog.empty()) {
            Populate_Object_Catalog<StructType, BuildingTypeClass>(BuildingsCatalog);

            BuildingListPager.ItemCount = BuildingsCatalog.size();
        }

        Render_Object_List(BUILDING_OBJECT_LIST, BuildingsCatalog, BuildingListPager);
    }

    if (!HelpText.has_value()) {
        return;
    }

    // help text for current object
    Fancy_Text_Print(TXT_NONE, 0, 0, BLACK, BLACK, TPF_MAP | TPF_NOSHADOW);
    const auto text_width = String_Pixel_Width(*HelpText);

    // shift help text location so the text doesn't hit the screen edge (and wrap)
    if (HelpTextX + text_width + 1 >= (X + W) - ControlMargin) {
        HelpTextX = (X + W) - text_width - ControlMargin;
    }

    Fancy_Text_Print(*HelpText, HelpTextX, HelpTextY, CC_GREEN, BLACK, TPF_MAP | TPF_NOSHADOW);
    LogicPage->Draw_Rect(HelpTextX - 1, HelpTextY - 1, HelpTextX + text_width + 1, HelpTextY + FontHeight, CC_GREEN);
}

bool MapEditorSidebar::On_Input(const KeyNumType& input, const bool forced)
{
    if (Parent == nullptr) {
        return false;
    }

    if (!forced) {
        if (Get_Mouse_X() < X || Get_Mouse_X() > X + W || Get_Mouse_Y() < Y || Get_Mouse_Y() > Y + H) {
            // input happened outside sidebar bounds, so ignore it
            return false;
        }
    }

    static const std::map<SidebarControls, int> button_to_content = {
        {OVERLAY_BUTTON, OVERLAY_GRID},
        {TERRAIN_BUTTON, TERRAIN_OBJECT_LIST},
        {UNITS_BUTTON, UNITS_GRID},
        {BUILDINGS_BUTTON, BUILDING_OBJECT_LIST},
        {TRIGGERS_BUTTON, TRIGGERS_LIST},
        {WAYPOINTS_BUTTON, WAYPOINTS_LIST}
    };

    const auto set_current_object_for_mouse = [&](const std::vector<ObjectCatalogItem>& catalog) {
        const auto mouse_x = Get_Mouse_X();
        const auto mouse_y = Get_Mouse_Y();

        for (const auto& [dimensions, object_type] : catalog) {
            if (!dimensions.Point_Is_Inside_Dimensions(mouse_x, mouse_y)) {
                continue;
            }

            if (CurrentObject == object_type) {
                // still hovering over the same object
                return;
            }

            CurrentObject = object_type;

            if (TdSettings.Display_Object_Icons() && object_type->Get_Cameo_Data()) {
                // we want icons and this object type has one, so help text is unnecessary
                return;
            }

            HelpText = object_type->Full_Name() != TXT_NONE
                ? Text_String(object_type->Full_Name())
                : object_type->IniName;
            HelpTextX = dimensions.X;
            HelpTextY = dimensions.Y;

            Parent->Flag_To_Redraw(true);
            return;
        }

        CurrentObject = nullptr;
        HelpText.reset();
    };

    switch (input) {
        // toggle buttons to switch content panels

        case (OVERLAY_BUTTON | KN_BUTTON):
        case (TERRAIN_BUTTON | KN_BUTTON):
        case (UNITS_BUTTON | KN_BUTTON):
        case (BUILDINGS_BUTTON | KN_BUTTON):
        case (WAYPOINTS_BUTTON | KN_BUTTON):
        case (TRIGGERS_BUTTON | KN_BUTTON): {
            for (auto i = OVERLAY_BUTTON; i <= TRIGGERS_BUTTON; ++i) {
                auto& control = Get_Control<TextButtonClass>(i);

                control.IsPressed = input == (i | KN_BUTTON);

                if (control.IsPressed) {
                    // show
                    Controls[button_to_content.at(i)]->Enable();

                    // disable all optional buttons
                    for (auto j = PREVIOUS_BUTTON; j <= DELETE_TRIGGER_BUTTON; ++j) {
                        Controls[j]->Disable(true);
                    }

                    if (i == WAYPOINTS_BUTTON ) {
                        Controls[CLEAR_WAYPT_BUTTON]->Enable();
                        Controls[GOTO_WAYPT_BUTTON]->Enable();
                    } else if (i == TRIGGERS_BUTTON) {
                        Controls[ADD_TRIGGER_BUTTON]->Enable();
                        Controls[EDIT_TRIGGER_BUTTON]->Enable();
                        Controls[DELETE_TRIGGER_BUTTON]->Enable();
                    } else {
                        Controls[PREVIOUS_BUTTON]->Enable();
                        Controls[NEXT_BUTTON]->Enable();
                    }
                } else {
                    // hide and prevent drawing
                    Controls[button_to_content.at(i)]->Disable(true);
                }
            }

            Parent->Flag_To_Redraw();
            break;
        }

        // start placement when content panel clicked

        case (OVERLAY_GRID | KN_BUTTON):
        case (TERRAIN_OBJECT_LIST | KN_BUTTON):
        case (UNITS_GRID | KN_BUTTON):
        case (BUILDING_OBJECT_LIST | KN_BUTTON):
            if (CurrentObject != nullptr) {
                Parent->Manual_Start_Placement(CurrentObject);
            }
            break;

        // content panel navigation

        case (PREVIOUS_BUTTON | KN_BUTTON):
            if (Get_Control<OVERLAY_GRID, ControlClass>().IsEnabled()) {
                OverlayGridPager.CurrentPage = max(0, OverlayGridPager.CurrentPage - 1);
            } else if (Get_Control<TERRAIN_OBJECT_LIST, ControlClass>().IsEnabled()) {
                if (TerrainPager.CurrentDepth > 0) {
                    TerrainPager.CurrentDepth--;
                    TerrainPager.CurrentIndex = TerrainPager.PageStartIndexes[TerrainPager.CurrentDepth];
                }
            } else if (Get_Control<UNITS_GRID, ControlClass>().IsEnabled()) {
                UnitsGridPager.CurrentPage = max(0, UnitsGridPager.CurrentPage - 1);
            } else if (Get_Control<BUILDING_OBJECT_LIST, ControlClass>().IsEnabled()) {
                if (BuildingListPager.CurrentDepth > 0) {
                    BuildingListPager.CurrentDepth--;
                    BuildingListPager.CurrentIndex = BuildingListPager.PageStartIndexes[BuildingListPager.CurrentDepth];
                }
            }

            Parent->Flag_To_Redraw();
            break;

        case (NEXT_BUTTON | KN_BUTTON):
            if (Get_Control<OVERLAY_GRID, ControlClass>().IsEnabled()) {
                OverlayGridPager.CurrentPage = min(OverlayGridPager.PageCount - 1, OverlayGridPager.CurrentPage + 1);
            } else if (Get_Control<TERRAIN_OBJECT_LIST, ControlClass>().IsEnabled()) {
                if (TerrainPager.CurrentDepth + 1 < TerrainPager.PageStartIndexes.size()) {
                    TerrainPager.CurrentDepth++;
                    TerrainPager.CurrentIndex = TerrainPager.PageStartIndexes[TerrainPager.CurrentDepth];
                }
            } else if (Get_Control<UNITS_GRID, ControlClass>().IsEnabled()) {
                UnitsGridPager.CurrentPage = min(UnitsGridPager.PageCount - 1 , UnitsGridPager.CurrentPage + 1);
            } else if (Get_Control<BUILDING_OBJECT_LIST, ControlClass>().IsEnabled()) {
                if (BuildingListPager.CurrentDepth + 1 < BuildingListPager.PageStartIndexes.size()) {
                    BuildingListPager.CurrentDepth++;
                    BuildingListPager.CurrentIndex = BuildingListPager.PageStartIndexes[BuildingListPager.CurrentDepth];
                }
            }

            Parent->Flag_To_Redraw();
            break;

        case (TRIGGERS_LIST | KN_BUTTON):
            Parent->Manual_Start_Trigger_Placement(
                CellTriggerList[Get_Control<TRIGGERS_LIST, ListClass>().Current_Index()]
            );
            break;

        case (ADD_TRIGGER_BUTTON | KN_BUTTON): {
            auto new_trigger = new TriggerClass();

            new_trigger->Event = EVENT_PLAYER_ENTERED;

            if (Parent->Edit_Trigger(new_trigger) == 0) {
                Parent->Mark_Changed();
                Parent->Manual_Start_Trigger_Placement(new_trigger);

                Refresh_Trigger_List();
            } else {
                delete new_trigger;
            }
            break;
        }

        case (EDIT_TRIGGER_BUTTON | KN_BUTTON): {
            auto selected_trigger = CellTriggerList[Get_Control<TRIGGERS_LIST, ListClass>().Current_Index()];

            if (Parent->Edit_Trigger(selected_trigger) == 0) {
                Parent->Mark_Changed();
                Parent->Cancel_Placement();

                Refresh_Trigger_List();
            }
            break;
        }

        case (DELETE_TRIGGER_BUTTON | KN_BUTTON): {
            auto selected_trigger = CellTriggerList[Get_Control<TRIGGERS_LIST, ListClass>().Current_Index()];

            selected_trigger->Remove();

            Parent->Mark_Changed();
            Parent->Cancel_Placement();

            Refresh_Trigger_List();
            break;
        }

        case (WAYPOINTS_LIST | KN_BUTTON): {
            const auto list_idx = Get_Control<WAYPOINTS_LIST, ListClass>().Current_Index();

            Parent->Start_Waypoint_Placement(WaypointLookup[list_idx]);
            break;
        }

        case (GOTO_WAYPT_BUTTON | KN_BUTTON): {
            const auto list_idx = Get_Control<WAYPOINTS_LIST, ListClass>().Current_Index();
            const auto waypoint_idx = WaypointLookup[list_idx];
            const auto& waypoint_cell = Scen.Waypoint[waypoint_idx];

            if (waypoint_cell != -1) {
                auto cell_x = Cell_X(waypoint_cell);
                auto cell_y = Cell_Y(waypoint_cell);

                // try to shift waypoint cell up and left so it is centered on screen when map snaps over to it
                const auto map_cell_width = Parent->TacLeptonWidth / CELL_LEPTON_W;
                const auto map_cell_height = Parent->TacLeptonHeight / CELL_LEPTON_H;

                cell_x = max(0, cell_x - (map_cell_width / 2));
                cell_y = max(0, cell_y - (map_cell_height / 2));

                Parent->DesiredTacticalCoord = Cell_Coord(XY_Cell(cell_x, cell_y));
                CurrentCell = waypoint_cell;

                Parent->Flag_To_Redraw(true);
            }

            Parent->Cancel_Placement();
            break;
        }

        case (CLEAR_WAYPT_BUTTON | KN_BUTTON): {
            const auto list_idx = Get_Control<WAYPOINTS_LIST, ListClass>().Current_Index();
            const auto waypoint_idx = WaypointLookup[list_idx];

            Scen.Waypoint[waypoint_idx] = -1;

            Parent->Mark_Changed();
            Parent->Flag_To_Redraw(true);

            Parent->Cancel_Placement();
            break;
        }

        default: {
            // automatically track current object based on mouse over event
            if (Get_Control<OVERLAY_GRID, ControlClass>().IsEnabled()) {
                set_current_object_for_mouse(OverlayCatalog);
            } else if (Get_Control<TERRAIN_OBJECT_LIST, ControlClass>().IsEnabled()) {
                set_current_object_for_mouse(TerrainCatalog);
            } else if (Get_Control<UNITS_GRID, ControlClass>().IsEnabled()) {
                set_current_object_for_mouse(UnitsCatalog);
            } else if (Get_Control<BUILDING_OBJECT_LIST, ControlClass>().IsEnabled()) {
                set_current_object_for_mouse(BuildingsCatalog);
            }

            return false;
        }
    }

    return true;
}
