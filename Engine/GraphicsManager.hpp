#pragma once

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include "GameConfig.hpp"
#include "Renderer.hpp"

class GraphicsManager {
public:
	GraphicsManager(UserGraphicsSettings settings, GLFWwindow* window);
	~GraphicsManager();
	std::vector<Renderer> getRenderers();
private:
	vk::Instance instance;
	vk::SurfaceKHR surface;
	std::vector<vk::PhysicalDevice> physicalDevices;
	UserGraphicsSettings settings;
};