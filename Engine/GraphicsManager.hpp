#pragma once

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define VK_USE_PLATFORM_WIN32_KHR
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_TYPESAFE_CONVERSION
#include <vulkan/vulkan.hpp>

#include "GameConfig.hpp"
#include "Renderer.hpp"

class GraphicsManager {
public:
	GraphicsManager(UserGraphicsSettings settings, GLFWwindow* window);
	std::vector<Renderer> getRenderers();
private:
	vk::UniqueInstance instance;
	vk::UniqueSurfaceKHR surface;
	std::vector<vk::PhysicalDevice> physicalDevices;
	UserGraphicsSettings settings;
};