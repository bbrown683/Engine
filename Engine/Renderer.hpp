#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include "GameConfig.hpp"

class Renderer {
public:
	Renderer(UserGraphicsSettings settings, vk::PhysicalDevice& physicalDevice, vk::UniqueSurfaceKHR& surface);
	~Renderer();
	bool updateSwapchain();
private:
	const char* physicalDeviceName;
	vk::Device device;
	vk::SwapchainKHR swapchain;
};