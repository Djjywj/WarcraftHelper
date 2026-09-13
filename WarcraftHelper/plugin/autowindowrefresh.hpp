#pragma once

#include "plugin.hpp"

class AutoWindowRefresh : IPlugin {
public:
        AutoWindowRefresh() = default;
        virtual void Start();
        virtual void Stop();
private:
        HANDLE thread = 0;
};
