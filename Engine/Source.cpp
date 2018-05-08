#include <GLFW/glfw3.h>
#include "Renderer.hpp"

enum class ErrorType {
	GlfwInitError		= -1,
	GlfwVulkanNotFound	= -2,
};

int main(int argc, char** argv) {
	int initResult = glfwInit();
	if (initResult != GLFW_TRUE)
		return static_cast<int>(ErrorType::GlfwInitError);

	if (!glfwVulkanSupported())
		return static_cast<int>(ErrorType::GlfwVulkanNotFound);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(1024, 768, "Engine", nullptr, nullptr);

	RenderingInstance instance(window);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}