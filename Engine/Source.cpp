#include <vector>
#include <GLFW/glfw3.h>

#include "Renderer.hpp"

int main(int argc, char** argv) {
	std::vector<const char*> args;
	args.insert(args.begin(), argv, argv + argc);

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(1024, 768, "Hello", nullptr, nullptr);

	Renderer renderer;
	if (!renderer.createRendererForWindow(window))
		return -1;

	auto gpus = renderer.enumerateGpus();
	if (!gpus.empty())
		renderer.selectGpu(gpus.front().id);
	else
		return -2;

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}