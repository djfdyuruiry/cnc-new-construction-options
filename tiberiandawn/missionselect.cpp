#include "common/ini.h"

#include "function.h"
#include "dialog.h"
#include "elist.h"

#ifdef NEWMENU

typedef enum
{
    BUTTON_OK = 200,
    BUTTON_CANCEL,
    BUTTON_ALL,
    BUTTON_GDI,
    BUTTON_NOD,
    BUTTON_FUNPARK,
    BUTTON_MISSIONS
} MissionSelectControls;

class MissionSelectDialog : public Dialog<MissionSelectControls>
{
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
        const ScenarioVarType& variation
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

        MissionCache.push_back({
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
    static void Fill_Mission_Cache()
    {
        MissionCache.clear();

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
                        Add_Mission_To_Cache_If_Present(player, mission, direction, variation);
                    }
                }
            }
        }
    }

    /**
     * Populate the list fields with all cached missions matching the MissionFilter.
     */
    void Populate_Mission_List()
    {
        auto& mission_list = Get_Control<BUTTON_MISSIONS, EListClass>();

        mission_list.Clear();
        MissionsToDisplay.clear();

        // add all missions matching filter values
        for (const auto& player : MissionFilter) {
            for (const auto& mission : MissionCache) {
                if (mission.Player == player) {
                    mission_list.Add_Item(mission.Description);
                    MissionsToDisplay.push_back(mission);
                }
            }
        }
    }

    void Reset_Tab_Styles()
    {
        for (auto control = BUTTON_ALL; control <= BUTTON_FUNPARK; ++control) {
            Get_Control<TextButtonClass>(control).IsPressed = true;
        }
    }

    void Change_Mission_Filter(const MissionSelectControls control, std::vector<ScenarioPlayerType> filter)
    {
        Reset_Tab_Styles();

        MissionFilter = std::move(filter);
        Populate_Mission_List();

        Get_Control<TextButtonClass>(control).IsPressed = false;
    }

    static inline std::vector<MissionVariables> MissionCache;

    std::vector<ScenarioPlayerType> MissionFilter;
    std::vector<MissionVariables> MissionsToDisplay;

protected:
    std::optional<bool> On_Input(DialogRedrawType& display, KeyNumType& input) override
    {
        switch (input) {
            case KN_RETURN:
            case BUTTON_OK | KN_BUTTON: {
                auto& list = Get_Control<BUTTON_MISSIONS, EListClass>();

                if (!list.Current_Item()) {
                    break;
                }

                const auto& [number, player, whom, dir, var, _] = MissionsToDisplay[list.Current_Index()];

                Scen.Scenario = number;
                ScenPlayer = player;
                Whom = whom;
                ScenDir = dir;
                ScenVar = var;
                Special.IsJurassic = player == SCEN_PLAYER_JP;
                AreThingiesEnabled = player == SCEN_PLAYER_JP;

                return true;
                break;
            }

            case KN_ESC:
            case BUTTON_CANCEL | KN_BUTTON: {
                Scen.Scenario = 1;
                ScenPlayer = SCEN_PLAYER_GDI;
                Whom = HOUSE_GOOD;
                ScenDir = SCEN_DIR_EAST;
                ScenVar = SCEN_VAR_A;
                Special.IsJurassic = false;
                AreThingiesEnabled = false;

                return false;
                break;
            }

            case BUTTON_ALL | KN_BUTTON:
                Change_Mission_Filter(BUTTON_ALL, { SCEN_PLAYER_GDI, SCEN_PLAYER_NOD, SCEN_PLAYER_JP });
                display = REDRAW_BACKGROUND;
                break;

            case BUTTON_GDI | KN_BUTTON:
                Change_Mission_Filter(BUTTON_GDI, { SCEN_PLAYER_GDI });
                display = REDRAW_BACKGROUND;
                break;

            case BUTTON_NOD | KN_BUTTON:
                Change_Mission_Filter(BUTTON_NOD, { SCEN_PLAYER_NOD });
                display = REDRAW_BACKGROUND;
                break;

            case BUTTON_FUNPARK | KN_BUTTON:
                Change_Mission_Filter(BUTTON_FUNPARK, { SCEN_PLAYER_JP });
                display = REDRAW_BACKGROUND;
                break;

            default:
                break;
        }

        return std::nullopt;
    }

    void Init_UI_State() override
    {
        // ensure the selected 'tab' is shown in bright text (the button which is not IsPressed)
        for (auto control = BUTTON_ALL; control <= BUTTON_FUNPARK; ++control) {
            auto& tab_button = Get_Control<TextButtonClass>(control);

            tab_button.Set_Gradient_Activated_Style(TPF_USE_GRAD_PAL | TPF_MEDIUM_COLOR);
            tab_button.Set_Gradient_Deactivated_Style(TPF_USE_GRAD_PAL | TPF_BRIGHT_COLOR);

            tab_button.IsPressed = control != BUTTON_ALL;
        }
    }

    void Init_Data() override
    {
        if (MissionCache.empty()) {
            Fill_Mission_Cache();
        }

        MissionFilter = { SCEN_PLAYER_GDI, SCEN_PLAYER_NOD, SCEN_PLAYER_JP };
        Populate_Mission_List();
    }

    void Init_Controls() override
    {
        Dialog::Init_Controls();

        Add_Button(BUTTON_OK, TXT_OK);
        Add_Button(BUTTON_CANCEL, TXT_CANCEL);
        Add_Button(BUTTON_ALL, "All"); // TODO: Locale string
        Add_Button(BUTTON_GDI, TXT_G_D_I);
        Add_Button(BUTTON_NOD, TXT_N_O_D);
        Add_Button(BUTTON_FUNPARK, "Funpark"); // TODO: Locale string

        Add_Control<BUTTON_MISSIONS, EListClass>(
            Dimensions[BUTTON_MISSIONS].X,
            Dimensions[BUTTON_MISSIONS].Y,
            Dimensions[BUTTON_MISSIONS].W,
            Dimensions[BUTTON_MISSIONS].H,
            TPF_6PT_GRAD | TPF_NOSHADOW,
            UpButtonShape,
            DownButtonShape
        );
    }

    void Init_Dimensions(const int screen_width, const int screen_height, const int factor) override
    {
        Dialog::Init_Dimensions(screen_width, screen_height, factor);

        Dimensions[BUTTON_OK].X = X + 25 * Factor;
        Dimensions[BUTTON_OK].Y = Y + Height - 15 * Factor;
        Dimensions[BUTTON_OK].W = -1;
        Dimensions[BUTTON_OK].H = -1;

        Dimensions[BUTTON_CANCEL].X = X + Width - 50 * Factor;
        Dimensions[BUTTON_CANCEL].Y = Y + Height - 15 * Factor;
        Dimensions[BUTTON_CANCEL].W = -1;
        Dimensions[BUTTON_CANCEL].H = -1;

        auto tab_x = X + 10 * Factor;

        for (auto control = BUTTON_ALL; control <= BUTTON_FUNPARK; ++control) {
            constexpr auto tab_width = 54;

            Dimensions[control].X = tab_x;
            Dimensions[control].Y = Y + 17 * Factor;
            Dimensions[control].W = tab_width * Factor;
            Dimensions[control].H = 8 * Factor;

            tab_x += tab_width * Factor;
        }

        Dimensions[BUTTON_MISSIONS].X = X + 10 * Factor;
        Dimensions[BUTTON_MISSIONS].Y = Y + 25 * Factor;
        Dimensions[BUTTON_MISSIONS].W = Width - 20 * Factor;
        Dimensions[BUTTON_MISSIONS].H = Height - 45 * Factor;
    }

public:
    MissionSelectDialog() : Dialog(236, 162)
    {
        CaptionText = "Campaign Selection";
    }

    ~MissionSelectDialog() override
    {
        if (!Controls.contains(BUTTON_MISSIONS)) {
            return;
        }

        const auto& list = Get_Control<BUTTON_MISSIONS, EListClass>();

        /*
        **	Free up the allocations for the text lines in the list box.
        */
        for (auto index = 0; index < list.Count(); index++) {
            delete[] const_cast<char*>(list.Get_Item(index));
        }
    }
};

bool Mission_Select_Dialog()
{
    const auto factor = (SeenBuff.Get_Width() == 320) ? 1 : 2;

    MissionSelectDialog dialog;

    dialog.Init(
        Try_Get_Resolution_Mode_Width().value_or(SeenBuff.Get_Width()),
        Try_Get_Resolution_Mode_Height().value_or(SeenBuff.Get_Height()),
        factor
    );

    const auto result = dialog.Present();

    // ensure correct data loaded if player picked a mission (ignore for demo mode)
    if (result && !Is_Demo()) {
        RequiredCD = ScenPlayer;

        if (!Force_CD_Available(ScenPlayer)) {
            Raise_Fatal_CD_Error(NAMEOF(Read_Scenario_Ini), ScenPlayer);
        }
    }

    return result;
}

#endif
