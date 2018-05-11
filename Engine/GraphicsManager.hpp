#pragma once

#include "GameConfig.hpp"

class GraphicsManager {
public:
	GraphicsManager(UserGraphicsSettings settings);
private:
	UserGraphicsSettings settings;
};