#pragma once
#include <string>
#include <thread>
#include <vector>
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_TYPESAFE_CONVERSION
#include <vulkan/vulkan.hpp>

typedef struct GLFWwindow GLFWwindow;

/// Instance-level Vulkan.
/// Creates instance, surface, and retreives available GPUs.
class RenderingInstance {
public:
	RenderingInstance(GLFWwindow*);
	vk::UniqueInstance& getInstance();
	std::vector<vk::PhysicalDevice>& getAllPhysicalDevices();
	vk::UniqueSurfaceKHR& getSurface();
private:
	std::vector<const char*> enabledExtensions;
	std::vector<const char*> enabledLayers;
	vk::UniqueInstance instance;
	std::vector<vk::PhysicalDevice> physicalDevices;
	vk::UniqueSurfaceKHR surface;
};

/// Device-level Vulkan.
/// Submits commands to specified GPU and window surface.
class RenderingDevice {
public:
	RenderingDevice(vk::PhysicalDevice&, vk::UniqueSurfaceKHR&);
	bool present();
	bool updateSwapchain();
	vk::UniqueDevice& getDevice();
	vk::UniqueSwapchainKHR& getSwapchain();
private:
	vk::PhysicalDevice& physicalDevice;
	vk::UniqueSurfaceKHR& surface;
	vk::UniqueDevice device;
	vk::UniqueSwapchainKHR swapchain;
	uint32_t threadCount;
};

/// Executes rendering commands in parallel from the main thread.
class ThreadedCommandPool {
public:
	ThreadedCommandPool(vk::UniqueDevice&);
private:
	std::thread thread;
	vk::UniqueCommandPool commandPool;
};