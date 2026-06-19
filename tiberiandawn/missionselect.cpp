#include "function.h"
#include "common/framelimit.h"
#include "common/ini.h"


class EListClass : public ListClass
{
public:
    EListClass(int id, int x, int y, int w, int h, TextPrintType flags, void const* up, void const* down)
        : ListClass(id, x, y, w, h, flags, up, down){};

    void Clear() {
        for (int i = 0; i < List.Count(); ++i) {
            if (List[i] != nullptr) {
                delete[] (char*)List[i];
            }
        }
        List.Clear();
        if (IsScrollActive && List.Count() <= LineCount) {
            Remove_Scroll_Bar();
        }
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
};

// lookups that use reference data from map selection screens to get the country name TXT value

static const auto gdi_country_name_lookup = [](const int& index) {
    if (index > 21) {
        return -1;
    }

    // first two GDI missions have country stored at the end of the lookup, so treat wrap around negative indexes
    if (index < 0) {
        return CountryNames[GDIStats[21].nameindex];
    }

    return CountryNames[GDIStats[index].nameindex];
};

static const auto nod_country_name_lookup = [](const int& index) {
    if (index > 22) {
        return -1;
    }

    return CountryNames[NodStats[index].nameindex];
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
 * present will be loaded into the mission list, with required metadata to enable correct loading of the mission.
 *
 * Country names are retrieved using the reference tables from map selection logic, if a scenario name is present in
 * the INI file it overrides country name.
 */
static void Populate_Mission_List(EListClass& mission_list, std::vector<MissionVariables>& mission_metadata, const std::vector<ScenarioPlayerType>& players)
{
    for (const auto& player : players) {
        /*
        ** Load mix files for GDI/NOD so we can enumerate scenario INI files.
        */
        RequiredCD = player;

        if (!Force_CD_Available(player)) {
            Raise_Fatal_CD_Error(NAMEOF(Read_Scenario_Ini), player);
        }

        const auto& country_name_lookup = player == SCEN_PLAYER_GDI
            ? gdi_country_name_lookup
            : nod_country_name_lookup;
        // workaround for gdi mission 1 + 2 being at then end of the countries lookup
        auto country_index = player == SCEN_PLAYER_GDI ? -2 : 0;

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

                        // read country name (if present)
                        const auto txt_country = country_name_lookup(
                            // workaround for duplicate gdi scenario 13 INI files (A + B variants are the same mission)
                            player == SCEN_PLAYER_GDI && mission > 13 ? country_index - 1 : country_index
                        );
                        std::optional<std::string> country;

                        // ignore country names for Funpark missions
                        if (player != SCEN_PLAYER_JP && txt_country > -1) {
                            country = Text_String(txt_country);
                        }

                        // only increment country index if not on final campaign mission
                        if (player == SCEN_PLAYER_GDI && mission < 15) {
                            ++country_index;
                        } else if (player == SCEN_PLAYER_NOD && mission < 13) {
                            ++country_index;
                        }

                        // store mission description and metadata for list logic
                        const auto description = Build_Mission_Description(
                            player, mission, direction, variation, name, country
                        );

                        mission_list.Add_Item(description);
                        mission_metadata.push_back({
                            mission,
                            player,
                            player == SCEN_PLAYER_GDI ? HOUSE_GOOD : HOUSE_BAD,
                            direction,
                            variation
                        });
                    }
                }
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

    TextButtonClass btn_all(203, "All", TPF_6PT_GRAD | TPF_NOSHADOW, option_x + 10 * factor, option_y + 12 * factor, 50 * factor, 8 * factor);
    TextButtonClass btn_gdi(204, "GDI", TPF_6PT_GRAD | TPF_NOSHADOW, option_x + 60 * factor, option_y + 12 * factor, 50 * factor, 8 * factor);
    TextButtonClass btn_nod(205, "NOD", TPF_6PT_GRAD | TPF_NOSHADOW, option_x + 110 * factor, option_y + 12 * factor, 50 * factor, 8 * factor);
    TextButtonClass btn_fun(206, "Funpark", TPF_6PT_GRAD | TPF_NOSHADOW, option_x + 160 * factor, option_y + 12 * factor, 60 * factor, 8 * factor);

    EListClass list(202,
                    option_x + 10 * factor,
                    option_y + 20 * factor,
                    option_width - 20 * factor,
                    option_height - 40 * factor,
                    TPF_6PT_GRAD | TPF_NOSHADOW,
                    up_button,
                    down_button);

    std::vector<MissionVariables> missions;
    std::vector<ScenarioPlayerType> current_filter = { SCEN_PLAYER_GDI, SCEN_PLAYER_NOD, SCEN_PLAYER_JP };

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
            Draw_Caption(TXT_MISSION_DESCRIPTION, option_x, option_y, option_width);
            buttons->Draw_All();
            Show_Mouse();
        }

        KeyNumType input = buttons->Input();
        switch (input) {
        case KN_RETURN:
        case 200 | KN_BUTTON:
            if (list.Current_Item()) {
                const auto& [number, player, whom, dir, var] = missions[list.Current_Index()];

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
            ScenVar = SCEN_VAR_NONE;
            Special.IsJurassic = false;
            AreThingiesEnabled = false;

            process = false;
            okval = false;
            break;

        case 203 | KN_BUTTON:
            current_filter = { SCEN_PLAYER_GDI, SCEN_PLAYER_NOD, SCEN_PLAYER_JP };
            list.Clear();
            missions.clear();
            Populate_Mission_List(list, missions, current_filter);
            list.Set_View_Index(0);
            display = true;
            break;
        case 204 | KN_BUTTON:
            current_filter = { SCEN_PLAYER_GDI };
            list.Clear();
            missions.clear();
            Populate_Mission_List(list, missions, current_filter);
            list.Set_View_Index(0);
            display = true;
            break;
        case 205 | KN_BUTTON:
            current_filter = { SCEN_PLAYER_NOD };
            list.Clear();
            missions.clear();
            Populate_Mission_List(list, missions, current_filter);
            list.Set_View_Index(0);
            display = true;
            break;
        case 206 | KN_BUTTON:
            current_filter = { SCEN_PLAYER_JP };
            list.Clear();
            missions.clear();
            Populate_Mission_List(list, missions, current_filter);
            list.Set_View_Index(0);
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

        if (!Force_CD_Available(ScenPlayer)) {
            Raise_Fatal_CD_Error(NAMEOF(Read_Scenario_Ini), ScenPlayer);
        }
    }

    return (okval);
}