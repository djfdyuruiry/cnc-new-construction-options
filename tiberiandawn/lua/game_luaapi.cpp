#include "function.h"

#include "game_luaapi.h"

void GameLuaApi::Register_Functions(LuaEngine& engine) const
{
    With_Api_Namespace(engine, [](auto& n) {
        n.addCFunction("win", [](auto L) {
            const auto engine = SharedLuaEngine(L);

            if (!PlayerPtr) {
                engine.Raise_Error("Attempted to lose game when PlayerPtr was null");
            }

            PlayerPtr->Flag_To_Win();

            return 0;
        })
        .addCFunction("lose", [](auto L) {
            const auto engine = SharedLuaEngine(L);

            if (!PlayerPtr) {
                engine.Raise_Error("Attempted to lose game when PlayerPtr was null");
            }

            PlayerPtr->Flag_To_Lose();

            return 0;
        });
    });
}
