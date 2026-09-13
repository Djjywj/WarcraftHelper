// Stub for WarcraftHelper/game/warcraft.hpp used only by the offline harness.
#pragma once

#include "../stub_win.h"
#include <stdint.h>

#define ERROR_GAMEDLL_INIT()
#define ERROR_GAMEWINDOW_INIT()

class System {
public:
    static BOOL IsKeyDown(int key) {
        short state = GetAsyncKeyState(key);
        return (state & 0x8000) ? true : false;
    }
};

class Game {
public:
    HWND GetGameWindow();
};

Game* GetGameInstance();
