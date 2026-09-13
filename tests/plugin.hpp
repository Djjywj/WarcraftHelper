// Stub for WarcraftHelper/plugin/plugin.hpp used only by the offline harness.
#pragma once

#include "game/warcraft.hpp"

class IPlugin {
public:
        IPlugin() = default;
        ~IPlugin() {};

        virtual void Start() = 0;
        virtual void Stop() = 0;
};
