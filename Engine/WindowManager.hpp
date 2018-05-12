#pragma once

#include <stdexcept>
#include <GLFW/glfw3.h>

#include "GameConfig.hpp"

class WindowManager {
public:
	WindowManager(UserWindowSettings settings);
	~WindowManager();
	GLFWwindow* getGlfwWindow();
private:
	GLFWwindow* window;
	UserWindowSettings settings;
};