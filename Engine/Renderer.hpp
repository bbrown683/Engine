#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_TYPESAFE_CONVERSION
#include <vulkan/vulkan.hpp>

#include "GameConfig.hpp"

class Renderer {
public:
	Renderer(UserGraphicsSettings settings, vk::PhysicalDevice& physicalDevice, vk::UniqueSurfaceKHR& surface);
	bool updateSwapchain();
private:
	const char* physicalDeviceName;
	vk::UniqueDevice device;
	vk::UniqueSwapchainKHR swapchain;
	void presentFrame();
};