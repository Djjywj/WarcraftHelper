#pragma once

#include "plugin.hpp"

class CursorLock : IPlugin {
public:
	CursorLock() = default;
	virtual void Start();
	virtual void Stop();
private:
	HANDLE thread = 0;
};
