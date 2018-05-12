#include "GameConfig.hpp"
#include "GraphicsManager.hpp"
#include "WindowManager.hpp"

int main(int argc, char** argv) {
	std::vector<const char*> args;
	args.insert(args.begin(), argv, argv + argc);

	GameConfig config;
	config.useDefaultSettings();

	WindowManager window(config.getUserWindowSettings());
	try {
		GraphicsManager graphics(config.getUserGraphicsSettings(), window.getGlfwWindow());
	}
	catch (std::exception e) {
		return -1;
	}

	while (!glfwWindowShouldClose(window.getGlfwWindow())) {
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}