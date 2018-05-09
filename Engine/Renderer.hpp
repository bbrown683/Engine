#pragma once

#if defined(__linux__)

#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#define VULKAN_HPP_TYPESAFE_CONVERSION
#include <vulkan/vulkan.hpp>

typedef struct GLFWwindow GLFWwindow;

class Renderer {
public:
	Renderer(GLFWwindow*, bool);
	void updateSwapchain();
	void present();
private:
	vk::PhysicalDevice& selectPhysicalDevice(std::vector<vk::PhysicalDevice>&);
	vk::Format selectSurfaceFormat(std::vector<vk::SurfaceFormatKHR>);

	vk::UniqueInstance instance;
	vk::UniqueSurfaceKHR surface;
	std::vector<const char*> availableExtensions;
	std::vector<const char*> availableLayers;
	std::vector<const char*> enabledExtensions;
	std::vector<const char*> enabledLayers;

	uint8_t selectedPhysicalDeviceIndex;
	std::vector<vk::PhysicalDevice> physicalDevices;
	std::vector<vk::PhysicalDeviceFeatures> physicalDeviceFeatures;
	std::vector<vk::PhysicalDeviceProperties> physicalDeviceProperties;
	std::vector<std::vector<vk::QueueFamilyProperties>> physicalDeviceQueueFamilies;
	std::vector<vk::SurfaceCapabilitiesKHR> physicalDeviceSurfaceCapabilities;
	std::vector<std::vector<vk::SurfaceFormatKHR>> physicalDeviceSurfaceFormats;
	std::vector<std::vector<vk::PresentModeKHR>> physicalDeviceSurfacePresentModes;

	vk::UniqueDevice device;
	vk::UniqueSwapchainKHR swapchain;
};