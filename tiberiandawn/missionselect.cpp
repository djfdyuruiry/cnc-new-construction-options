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

// TODO: lookup point of conflict (see mapsel.cpp)
// TODO: Add rule for disabling country lookups, so custom campaigns can hide default names
static std::optional<std::string> Lookup_Country(const ScenarioPlayerType& player, const std::string& ini_name)
{
    // calculating country index fails if levels are missing/new ones added etc. - so we just do a lookup instead
    static const std::map<ScenarioPlayerType, std::map<std::string, int>> country_index_lookups = {
        { SCEN_PLAYER_GDI, {
            { "SCG01EA.INI", 0 },
            { "SCG02EA.INI", 1 },
            { "SCG03EA.INI", 2 },
            { "SCG04EA.INI", 3 },
            { "SCG04WA.INI", 4 },
            { "SCG04WB.INI", 5 },
            { "SCG05EA.INI", 6 },
            { "SCG05EB.INI", 7 },
            { "SCG05WA.INI", 8 },
            { "SCG05WB.INI", 9 },
            { "SCG06EA.INI", 10 },
            { "SCG07EA.INI", 11 },
            { "SCG08EA.INI", 12 },
            { "SCG08EB.INI", 13 },
            { "SCG09EA.INI", 14 },
            { "SCG10EA.INI", 15 },
            { "SCG10EB.INI", 16 },
            { "SCG11EA.INI", 17 },
            { "SCG12EA.INI", 18 },
            { "SCG12EB.INI", 19 },
            { "SCG13EA.INI", 20 },
            { "SCG13EB.INI", 21 },
            { "SCG14EA.INI", 21 },
            { "SCG15EA.INI", 22 },
            { "SCG15EB.INI", 22 },
            { "SCG15EC.INI", 22 }
            }
        }, { SCEN_PLAYER_NOD, {
            { "SCB01EA.INI", 0 },
            { "SCB02EA.INI", 1 },
            { "SCB02EB.INI", 2 },
            { "SCB03EA.INI", 3 },
            { "SCB03EB.INI", 4 },
            { "SCB04EA.INI", 5 },
            { "SCB04EB.INI", 6 },
            { "SCB05EA.INI", 7 },
            { "SCB06EA.INI", 8 },
            { "SCB06EB.INI", 9 },
            { "SCB06EC.INI", 10 },
            { "SCB07EA.INI", 11 },
            { "SCB07EB.INI", 12 },
            { "SCB07EC.INI", 13 },
            { "SCB08EA.INI", 14 },
            { "SCB08EB.INI", 15 },
            { "SCB09EA.INI", 16 },
            { "SCB10EA.INI", 17 },
            { "SCB10EB.INI", 18 },
            { "SCB11EA.INI", 19 },
            { "SCB11EB.INI", 20 },
            { "SCB12EA.INI", 21 },
            { "SCB13EA.INI", 22 },
            { "SCB13EB.INI", 22 },
            { "SCB13EC.INI", 22 }
            }
        }
    };

    const auto player_country_index_lookup = player != SCEN_PLAYER_JP
        ? &country_index_lookups.at(player)
        : nullptr;

    if (player_country_index_lookup == nullptr || !player_country_index_lookup->contains(ini_name)) {
        return std::nullopt;
    }

    // fetch country name for INI name
    const auto country_txt = Lookup_Country_Name(player, player_country_index_lookup->at(ini_name));

    if (country_txt == TXT_NONE) {
        return std::nullopt;
    }

    return Text_String(country_txt);;
}

/**
 * Build a human-readable description for a given mission using its metadata - country name, direction and variation
 * are used.
 *
 * Country names are retrieved using the reference tables from map selection logic.
 *
 * If mission_name is present, it is preferred over the above metadata, so setting the [Basic] -> Name field in the
 * scenario INI is a way to set your own descriptions for custom campaigns.
 */
static std::string Build_Mission_Description(
    const ScenarioPlayerType& player,
    const int& scenario_number,
    const ScenarioDirType& direction,
    const ScenarioVarType& variation,
    const std::string& ini_name,
    const std::optional<std::string>& mission_name
)
{
    auto mission_description = mission_name.value_or("");

    // if no mission name was found in the INI files, build a default description using metadata
    if (!mission_name.has_value()) {
        const auto country = Lookup_Country(player, ini_name);
        auto country_name_str = country.value_or("");
        auto direction_str = TdTypeConverter::To_String(direction);

        CncStringUtils::To_Title_Case(country_name_str);
        CncStringUtils::To_Title_Case(direction_str);

        // {country name} {direction} ({variation})
        mission_description = country_name_str + (country.has_value() ? " " : "") +
            std::format("({} {})", direction_str, variation);
    }

    // {campaign player}: Mission {number} - {mission description}
    // (Note: 'Mission ' is omitted when running in DOS mode to ensure description fits EListClass control bounds)
    return std::format(
        "{}{}:{} {:>2} - {}",
        std::string(sizeof(int), ' '), // leading spaces are to maintain compatibility with drawing logic
        player == SCEN_PLAYER_JP ? "Funpark" : TdTypeConverter::To_String(player),
        Get_Resolution_Factor() == 0 ? "" : " Mission", // shorten mission description for DOS resolution
        scenario_number,
        mission_description
    );
}

static void Add_Mission_To_Cache_If_Present(
    const ScenarioPlayerType& player,
    const int& mission,
    const ScenarioDirType& direction,
    const ScenarioVarType& variation,
    std::vector<MissionVariables>& mission_cache
)
{
    CCFileClass file;
    char ini_filename[128];

    Set_Scenario_Name(ini_filename, mission, player, direction, variation);
    strcat(ini_filename, ".INI");
    file.Set_Name(ini_filename);

    CCINIClass ini;

    if (!ini.Load(file, true)) {
        return;
    }

    // read custom scenario name (if present)
    static const std::string no_name = "<none>";
    const auto ini_name = ini.Get_String("Basic", "Name", no_name);
    const auto name = ini_name == no_name ? std::nullopt : std::optional(ini_name);

    // store mission description and metadata for list logic
    auto description = Build_Mission_Description(
        player, mission, direction, variation, ini_filename, name
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

/**
 * Find any missions between 1-20 for all three single player campaigns (GDI, NOD and Funpark). Any scenario INI files
 * present will be loaded into the mission cache, containing required metadata to enable correct loading of the mission.
 */
static void Fill_Mission_Cache(std::vector<MissionVariables>& mission_cache)
{
    for (const auto& player : { SCEN_PLAYER_GDI, SCEN_PLAYER_NOD, SCEN_PLAYER_JP }) {
        /*
        ** Attempt to load mix files for GDI/NOD so we can enumerate scenario INI files.
        ** Failure is ignored so we can populate as many missions as possible regardless of available data.
        */
        const auto old_cd = RequiredCD;
        RequiredCD = player;

        if (!Force_CD_Available(player)) {
            CNC_LOG_WARN("Missing CD data detected when looking for {} missions", player);
            RequiredCD = old_cd;
        }

        // TODO: Add rules for constraining mission number scan, so custom campaigns can hide default scenarios
        for (auto mission = 1; mission < 20; mission++) {
            for (const auto& direction : { SCEN_DIR_EAST, SCEN_DIR_WEST }) {
                for (auto variation = SCEN_VAR_A; variation < SCEN_VAR_COUNT; ++variation) {
                    Add_Mission_To_Cache_If_Present(player, mission, direction, variation, mission_cache);
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

    // add all missions matching filter values
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

    // ensure correct data loaded if player picked a mission (ignore for demo mode)
    if (okval && !Is_Demo()) {
        RequiredCD = ScenPlayer;

        if (!Force_CD_Available(ScenPlayer)) {
            Raise_Fatal_CD_Error(NAMEOF(Read_Scenario_Ini), ScenPlayer);
        }
    }

    return (okval);
}

#endif
