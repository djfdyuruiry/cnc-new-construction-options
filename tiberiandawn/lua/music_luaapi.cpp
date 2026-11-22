#include "common/lua/luaarguments.h"
#include "common/lua/luatablebuilder.h"

#include "function.h"
#include "music_luaapi.h"

void MusicLuaApi::Register_Functions(LuaEngine& engine) const
{
    With_Api_Namespace(engine, [](auto& n) {
        n.addCFunction("play", [](auto L) {
            const auto e = SharedLuaEngine(L);
            auto arguments = LuaArguments(
                e,
                "play(<string: trackName>)"
            );

            arguments.Count_Is(1).First_Argument_Is<std::string>().Assert();

            const auto name = arguments.Read_First<std::string>().Unpack();

            const auto theme = TdTypeConverter::Try_Parse<ThemeType>(name);

            if (!theme.has_value()) {
                e.Raise_Error_Format(
                    "Unknown music track: {} (see availableTracks() for valid names)",
                    name
                );
            }

            Theme.Queue_Song(*theme);

            return 0;
        })
        .addCFunction("stop", [](auto L) {
            Theme.Queue_Song(THEME_NONE);

            return 0;
        })
        .addCFunction("getTrack", [](auto L) {
            const auto e = SharedLuaEngine(L);

            const auto current = Theme.What_Is_Playing();

            if (current == THEME_NONE) {
                return 0;
            }

            e.Push_Value(TdTypeConverter::To_String<ThemeType>(current));

            return 1;
        })
        .addCFunction("availableTracks", [](auto L) {
            const auto e = SharedLuaEngine(L);

            auto table = LuaTableBuilder::Push_New_Table(e);

            for (const auto& name : TdTypeConverter::Get_Valid_Strings<ThemeType>()) {
                table.With_Index_Value(name);
            }

            return 1;
        })
        .addCFunction("getDisplayName", [](auto L) {
            const auto e = SharedLuaEngine(L);
            auto arguments = LuaArguments(
                e,
                "getDisplayName(<string: trackName>)"
            );

            arguments.Count_Is(1).First_Argument_Is<std::string>().Assert();

            const auto name = arguments.Read_First<std::string>().Unpack();

            const auto theme = TdTypeConverter::Try_Parse<ThemeType>(name);

            if (!theme.has_value()) {
                e.Raise_Error_Format(
                    "Unknown music track: {} (see availableTracks() for valid names)",
                    name
                );
            }

            const auto display_name = Theme.Full_Name(*theme);

            if (display_name == nullptr) {
                return 0;
            }

            e.Push_Value(std::string(display_name));

            return 1;
        });
    });
}
