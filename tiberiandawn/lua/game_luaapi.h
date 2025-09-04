#pragma once

#include <memory>

#include "../common/lua/luaapi.h"

#include "../externs.h"

#include "td_luaevents.h"

class GameLuaApi: public LuaApi {
public:
    GameLuaApi() : LuaApi("Game", { "Game.lua" }) {}

    virtual void Register_Functions(LuaEngine& engine) const override {
        With_Api_Namespace(engine, [](auto& n) {
            n.addCFunction("win", [](auto L) {
                auto engine = SharedLuaEngine(L);

                if (!PlayerPtr) {
                    engine.Raise_Error("Attempted to lose game when PlayerPtr was null");
                }

                PlayerPtr->Flag_To_Win();

                return 0;
            })
            .addCFunction("lose", [](auto L) {
                auto engine = SharedLuaEngine(L);

                if (!PlayerPtr) {
                    engine.Raise_Error("Attempted to lose game when PlayerPtr was null");
                }

                PlayerPtr->Flag_To_Lose();

                return 0;
            });
        });
    }

protected:
    virtual const char* Get_Cpp_Source() const override {
        return __FILE__;
    }

};
