#include "game/warcraft.hpp"
#include "config/config.hpp"

Config config;
Config* GetConfig() { return &config; }

static Game g_gameInstance;

Game* GetGameInstance() { return &g_gameInstance; }

HWND Game::GetGameWindow() {
    g_getGameWindowCalls++;
    return g_game;
}
