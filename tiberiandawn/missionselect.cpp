#include "function.h"
#include "common/framelimit.h"
#include "common/ini.h"

#ifdef NEWMENU

class EListClass : public ListClass
{
public:
    EListClass(int id, int x, int y, int w, int h, TextPrintType flags, void const* up, void const* down)
        : ListClass(id, x, y, w, h, flags, up, down){};

    void Clear() {
        List.Clear();

        if (IsScrollActive) {
            Remove_Scroll_Bar();
        }

        Set_Selected_Index(0);
    }

protected:
    void Draw_Entry(int index, int x, int y, int width, int selected) override
    {
        if (TextFlags & TPF_6PT_GRAD) {
            TextPrintType flags = TextFlags;

            if (selected) {
                flags = flags | TPF_BRIGHT_COLOR;
                LogicPage->Fill_Rect(x, y, x + width - 1, y + LineHeight - 1, CC_GREEN_SHADOW);
            } else {
                if (!(flags & TPF_USE_GRAD_PAL)) {
                    flags = flags | TPF_MEDIUM_COLOR;
                }
            }

            Conquer_Clip_Text_Print(List[index], x, y, CC_GREEN, TBLACK, flags, width, Tabs);

        } else {
            Conquer_Clip_Text_Print(
                List[index], x, y, (selected ? BLUE : WHITE), TBLACK, TextFlags, width, Tabs);
        }
    }
};

struct MissionVariables
{
    int Number;
    ScenarioPlayerType Player;
    HousesType House;
    ScenarioDirType Direction;
    ScenarioVarType Variant;
    std::string Description;
};


/**
 * Build a human readable description for a given mission using it's metadata.
 */
static std::string Build_Mission_Description(
    const ScenarioPlayerType player,
    const int scenario_number,
    const ScenarioDirType direction,
    const ScenarioVarType variation,
    const std::optional<std::string>& mission_name,
    const std::optional<std::string>& country
)
{
    auto mission_description = mission_name.value_or("");

    // if no mission name was found in the INI files, build a default description using metadata
    if (!mission_name.has_value()) {
        auto country_name_str = country.value_or("");
        auto direction_str = TdTypeConverter::To_String(direction);

        CncStringUtils::To_Title_Case(country_name_str);
        CncStringUtils::To_Title_Case(direction_str);

        // {country name} {direction} ({variation})
        mission_description =
            country_name_str + (country.has_value() ? " " : "") +
            std::format("({} {})", direction_str, TdTypeConverter::To_String(variation))
        ;
    }

    // {campaign player}: Mission {number} - {mission description}
    return std::format(
        "{}{}: Mission {:>2} - {}",
        std::string(sizeof(int), ' '), // leading spaces are to maintain compatibility with drawing logic
        player == SCEN_PLAYER_JP ? "Funpark" : TdTypeConverter::To_String(player),
        scenario_number,
        mission_description
    );
}

/**
 * Find any missions between 1-20 for all three single player campaigns (GDI, NOD and Funpark). Any scenario INI
 * present will be loaded into the mission cache, containing required metadata to enable correct loading of the mission.
 *
 * Country names are retrieved using the reference tables from map selection logic, if a scenario name is present in
 * the INI file it overrides country name.
 */
static void Fill_Mission_Cache(std::vector<MissionVariables>& mission_cache)
{
    for (const auto& player : { SCEN_PLAYER_GDI, SCEN_PLAYER_NOD, SCEN_PLAYER_JP }) {
        /*
        ** Attempt to load mix files for GDI/NOD so we can enumerate scenario INI files.
        */
        auto old_cd = RequiredCD;
        RequiredCD = player;

        if (!Force_CD_Available(player)) {
            RequiredCD = old_cd;
        }

        auto country_index = 0;

        // TODO: lookup point of conflict (see mapsel.cpp)
        for (auto mission = 1; mission < 20; mission++) {
            for (const auto& direction : { SCEN_DIR_EAST, SCEN_DIR_WEST }) {
                for (auto variation = SCEN_VAR_A; variation < SCEN_VAR_COUNT; ++variation) {
                    CCFileClass file;
                    char buffer[128];

                    Set_Scenario_Name(buffer, mission, player, direction, variation);
                    strcat(buffer, ".INI");
                    file.Set_Name(buffer);

                    if (CCINIClass ini; ini.Load(file, true)) {
                        // read custom scenario name (if present)
                        static const std::string no_name = "<none>";
                        auto ini_name = ini.Get_String("Basic", "Name", no_name);
                        auto name = ini_name == no_name ? std::nullopt : std::optional(ini_name);

                        // workaround for duplicate gdi scenario 13 INI files (A + B variants are the same mission)
                        if (
                            player == SCEN_PLAYER_GDI && mission == 14
                        ) {
                            country_index = 21;
                        }

                        // read country name (if present)
                        const auto txt_country = Lookup_Country_Name(player, country_index);

                        CNC_LOG_WARN("index: {} | mission: {} | txt_country: {}", country_index, buffer, txt_country);

                        std::optional<std::string> country;

                        if (txt_country != TXT_NONE) {
                            country = Text_String(txt_country);
                        }

                        if (player == SCEN_PLAYER_GDI && mission < 15) {
                            ++country_index;
                        } else if (player == SCEN_PLAYER_NOD && mission < 13) {
                            ++country_index;
                        }

                        // store mission description and metadata for list logic
                        auto description = Build_Mission_Description(
                            player, mission, direction, variation, name, country
                        );

                        mission_cache.push_back({
                            mission,
                            player,
                            player == SCEN_PLAYER_GDI ? HOUSE_GOOD : HOUSE_BAD,
                            direction,
                            variation,
                            std::move(description)
                        });
                    }
                }
            }
        }
    }
}

/**
 * Populate the given list variables with all cached missions matching the players_filter.
 */
static void Populate_Mission_List(
    EListClass& mission_list,
    std::vector<MissionVariables const*>& mission_metadata,
    const std::vector<ScenarioPlayerType>& players_filter
)
{
    static std::vector<MissionVariables> mission_cache;

    if (mission_cache.empty()) {
        Fill_Mission_Cache(mission_cache);
    }

    for (const auto& player : players_filter) {
        for (const auto& mission : mission_cache) {
            if (mission.Player == player) {
                mission_list.Add_Item(mission.Description);
                mission_metadata.push_back(&mission);
            }
        }
    }
}

bool Mission_Select_Dialog(void)
{
    int factor = (SeenBuff.Get_Width() == 320) ? 1 : 2;

    int option_width = 236 * factor;
    int option_height = 162 * factor;
    int option_x = (Try_Get_Resolution_Mode_Width().value_or(SeenBuff.Get_Width()) - option_width) / 2;
    int option_y = (Try_Get_Resolution_Mode_Height().value_or(SeenBuff.Get_Height()) - option_height) / 2;

    GadgetClass* buttons = NULL;

    void const* up_button;
    void const* down_button;

    if (InMainLoop || factor == 1) {
        up_button = Hires_Retrieve("BTN-UP.SHP");
        down_button = Hires_Retrieve("BTN-DN.SHP");
    } else {
        up_button = Hires_Retrieve("BTN-UP2.SHP");
        down_button = Hires_Retrieve("BTN-DN2.SHP");
    }

    TextButtonClass ok(
        200, TXT_OK, TPF_6PT_GRAD | TPF_NOSHADOW, option_x + 25 * factor, option_y + option_height - 15 * factor);
    TextButtonClass cancel(201,
                            TXT_CANCEL,
                            TPF_6PT_GRAD | TPF_NOSHADOW,
                            option_x + option_width - 50 * factor,
                            option_y + option_height - 15 * factor);

    // TODO: Locale file entry
    TextButtonClass btn_all(203, "All", TPF_6PT_GRAD | TPF_NOSHADOW, option_x + 10 * factor, option_y + 17 * factor, 54 * factor, 8 * factor);
    TextButtonClass btn_gdi(204, Text_String(TXT_G_D_I), TPF_6PT_GRAD | TPF_NOSHADOW, option_x + 64 * factor, option_y + 17 * factor, 54 * factor, 8 * factor);
    TextButtonClass btn_nod(205, Text_String(TXT_N_O_D), TPF_6PT_GRAD | TPF_NOSHADOW, option_x + 118 * factor, option_y + 17 * factor, 54 * factor, 8 * factor);
    // TODO: Locale file entry
    TextButtonClass btn_fun(206, "Funpark", TPF_6PT_GRAD | TPF_NOSHADOW, option_x + 172 * factor, option_y + 17 * factor, 54 * factor, 8 * factor);

    // setup helpers for managing the buttons as 'tabs'
    const std::vector button_ptrs = { &btn_all, &btn_gdi, &btn_nod, &btn_fun };

    // ensure the selected 'tab' is shown in bright text (the button which is not IsPressed)
    for (const auto& button : button_ptrs) {
        button->Set_Gradient_Activated_Style(TPF_USE_GRAD_PAL | TPF_MEDIUM_COLOR);
        button->Set_Gradient_Deactivated_Style(TPF_USE_GRAD_PAL | TPF_BRIGHT_COLOR);
    }

    // press all button to make them 'tabs' in the background
    const auto reset_button_styles = [&]() {
        std::ranges::for_each(button_ptrs, [](auto& b) { b->IsPressed = true; });
    };

    // select 'All' tab by default
    reset_button_styles();
    btn_all.IsPressed = false;

    EListClass list(202,
                    option_x + 10 * factor,
                    option_y + 25 * factor,
                    option_width - 20 * factor,
                    option_height - 45 * factor,
                    TPF_6PT_GRAD | TPF_NOSHADOW,
                    up_button,
                    down_button);

    std::vector<MissionVariables const*> missions;
    std::vector current_filter = { SCEN_PLAYER_GDI, SCEN_PLAYER_NOD, SCEN_PLAYER_JP };

    Populate_Mission_List(list, missions, current_filter);

    buttons = &ok;
    cancel.Add(*buttons);
    list.Add(*buttons);
    btn_all.Add(*buttons);
    btn_gdi.Add(*buttons);
    btn_nod.Add(*buttons);
    btn_fun.Add(*buttons);

    Set_Logic_Page(SeenBuff);
    bool recalc = true;
    bool display = true;
    bool process = true;
    bool okval = true;
    while (process) {

        Call_Back();

        /*
        ** If we have just received input focus again after running in the background then
        ** we need to redraw.
        */
        if (AllSurfaces.SurfacesRestored) {
            AllSurfaces.SurfacesRestored = false;
            display = true;
        }

        if (display) {
            display = false;

            Hide_Mouse();

            /*
            **	Load the background picture.
            */
            Load_Title_Screen(TitlePicture, &HidPage, Palette);
            Blit_Hid_Page_To_Seen_Buff();

            Dialog_Box(option_x, option_y, option_width, option_height);
            Draw_Caption("Campaign Selection", option_x, option_y, option_width); // TODO: Locale file entry
            buttons->Draw_All();
            Show_Mouse();
        }

        KeyNumType input = buttons->Input();
        switch (input) {
        case KN_RETURN:
        case 200 | KN_BUTTON:
            if (list.Current_Item()) {
                const auto& [number, player, whom, dir, var, _] = *(missions[list.Current_Index()]);

                Scen.Scenario = number;
                ScenPlayer = player;
                Whom = whom;
                ScenDir = dir;
                ScenVar = var;
                Special.IsJurassic = player == SCEN_PLAYER_JP;
                AreThingiesEnabled = player == SCEN_PLAYER_JP;

                okval = true;
                process = false;
                break;
            }

        case KN_ESC:
        case 201 | KN_BUTTON:
            Scen.Scenario = 1;
            ScenPlayer = SCEN_PLAYER_GDI;
            Whom = HOUSE_GOOD;
            ScenDir = SCEN_DIR_EAST;
            ScenVar = SCEN_VAR_A;
            Special.IsJurassic = false;
            AreThingiesEnabled = false;

            process = false;
            okval = false;
            break;

        case 203 | KN_BUTTON:
            reset_button_styles();
            list.Clear();
            missions.clear();

            current_filter = { SCEN_PLAYER_GDI, SCEN_PLAYER_NOD, SCEN_PLAYER_JP };
            Populate_Mission_List(list, missions, current_filter);

            btn_all.IsPressed = false;
            display = true;
            break;
        case 204 | KN_BUTTON:
            reset_button_styles();
            list.Clear();
            missions.clear();

            current_filter = { SCEN_PLAYER_GDI };
            Populate_Mission_List(list, missions, current_filter);

            btn_gdi.IsPressed = false;
            display = true;
            break;
        case 205 | KN_BUTTON:
            reset_button_styles();
            list.Clear();
            missions.clear();

            current_filter = { SCEN_PLAYER_NOD };
            Populate_Mission_List(list, missions, current_filter);

            btn_nod.IsPressed = false;
            display = true;
            break;
        case 206 | KN_BUTTON:
            reset_button_styles();
            list.Clear();
            missions.clear();

            current_filter = { SCEN_PLAYER_JP };
            Populate_Mission_List(list, missions, current_filter);

            btn_fun.IsPressed = false;
            display = true;
            break;

        default:
            break;
        }

        Frame_Limiter();
    }

    /*
    **	Free up the allocations for the text lines in the list box.
    */
    for (auto index = 0; index < list.Count(); index++) {
        delete[](char*) list.Get_Item(index);
    }

    // ensure correct data loaded if player picked a mission
    if (okval) {
        RequiredCD = ScenPlayer;

        if (!Force_CD_Available(ScenPlayer) && !Is_Demo()) {
            Raise_Fatal_CD_Error(NAMEOF(Read_Scenario_Ini), ScenPlayer);
        }
    }

    return (okval);
}

#endif
