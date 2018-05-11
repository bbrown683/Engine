#pragma once

#include "WindowManager.hpp"

class WindowManager {
public:
	WindowManager(UserWindowSettings settings);
private:
	UserWindowSettings settings;
};