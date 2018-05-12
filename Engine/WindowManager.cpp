#include "WindowManager.hpp"

WindowManager::WindowManager(UserWindowSettings settings) : settings(settings) {
	if (!glfwInit())
		throw std::runtime_error("CRITICAL: GLFW failed to initialize!");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(1024, 768, "Engine", nullptr, nullptr);
}

WindowManager::~WindowManager() {
	glfwTerminate();
}

GLFWwindow* WindowManager::getGlfwWindow() {
	return window;
}
