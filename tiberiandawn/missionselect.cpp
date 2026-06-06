#include "function.h"
#include "common/framelimit.h"
#include "common/ini.h"

#include <redalert/externs.h>

#ifdef NEWMENU

class EListClass : public ListClass
{
public:
    EListClass(int id, int x, int y, int w, int h, TextPrintType flags, void const* up, void const* down)
        : ListClass(id, x, y, w, h, flags, up, down){};

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

std::string Build_Mission_Description(
    const ScenarioPlayerType player,
    const int scenario_number,
    const ScenarioDirType direction,
    const ScenarioVarType variation,
    std::optional<std::string> mission_name
)
{
    if (!mission_name.has_value()) {
        auto direction_str = TdTypeConverter::To_String(direction);

        CncStringUtils::To_Title_Case(direction_str);

        mission_name = std::format(
            "{}{}({})",
            direction_str,
            direction == SCEN_DIR_EAST ? "  " : " ",
            TdTypeConverter::To_String(variation)
        );
    }

    return std::format(
        "{}: Mission {:>2} - {}",
        TdTypeConverter::To_String(player),
        scenario_number,
        mission_name.value()
    );

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
    EListClass list(202,
                    option_x + 10 * factor,
                    option_y + 20 * factor,
                    option_width - 20 * factor,
                    option_height - 40 * factor,
                    TPF_6PT_GRAD | TPF_NOSHADOW,
                    up_button,
                    down_button);

    struct MissionVariables
    {
        int Number;
        ScenarioPlayerType Player;
        HousesType House;
        ScenarioDirType Direction;
        ScenarioVarType Variant;
    };
    std::vector<MissionVariables> missions;

    buttons = &ok;
    cancel.Add(*buttons);
    list.Add(*buttons);

    /*
    **	Add in all the expansion scenarios.
    */
    for (const auto& player : { SCEN_PLAYER_GDI, SCEN_PLAYER_NOD }) {
        /*
        ** Load mix files for GDI/NOD so we can enumerate scenario INI files.
        */
        RequiredCD = player;

        if (!Force_CD_Available(player)) {
            Raise_Fatal_CD_Error(NAMEOF(Read_Scenario_Ini), player);
        }

        // TODO: Have between INI name and country (backed by INI file for modding/locale?)
        for (auto index = 1; index < 20; index++) {
            for (const auto& direction : { SCEN_DIR_EAST, SCEN_DIR_WEST }) {
                for (auto variation = SCEN_VAR_A; variation < SCEN_VAR_COUNT; ++variation) {
                    CCFileClass file;
                    char buffer[128];

                    Set_Scenario_Name(buffer, index, player, direction, variation);
                    strcat(buffer, ".INI");
                    file.Set_Name(buffer);

                    if (CCINIClass ini; ini.Load(file, true)) {
                        static const std::string no_name = "<none>";
                        auto ini_name = ini.Get_String("Basic", "Name", no_name);
                        auto name = ini_name == no_name ? std::nullopt : std::optional(ini_name);

                        const auto description = Build_Mission_Description(
                            player, index, direction, variation, name
                        );

                        list.Add_Item(description);

                        missions.push_back({
                            index,
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

            process = false;
            okval = false;
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

#endif