// Stub for WarcraftHelper/config/config.hpp used only by the offline harness.
#pragma once

struct Config {
    int m_autoRefreshInterval = 15;
};

extern Config config;
Config* GetConfig();
