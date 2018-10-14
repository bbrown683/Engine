#pragma once

#define NOMINMAX

#include <SDL2/SDL_config.h>
#include <SDL2/SDL_syswm.h>
#ifdef SDL_VIDEO_DRIVER_MIR
#define VK_USE_PLATFORM_MIR_KHR
#elif defined(SDL_VIDEO_DRIVER_WAYLAND)
#define VK_USE_PLATFORM_WAYLAND_KHR
#elif defined(SDL_VIDEO_DRIVER_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(SDL_VIDEO_DRIVER_X11)
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <vulkan/vulkan.hpp>

struct HelperVk {
public:
	static bool hasRequiredInstanceExtensions();
	static bool hasRequiredDeviceExtensionsAndFeatures(vk::PhysicalDevice physicalDevice);
	static vk::UniqueInstance createInstance(SDL_SysWMinfo wmInfo);
	static vk::UniqueSurfaceKHR createSurface(vk::Instance instance, SDL_SysWMinfo wmInfo);
	static vk::UniqueDevice createDevice(vk::PhysicalDevice physicalDevice, uint32_t queueIndex);
	static vk::UniqueSwapchainKHR createSwapchain(vk::Device device, vk::SurfaceKHR surface, vk::Extent2D extent,
		uint32_t numImages, vk::Format format = vk::Format::eR8G8B8A8Unorm, vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo,
		vk::SwapchainKHR previousSwapchain = nullptr);
	static std::vector<vk::UniqueImageView> createImageViews(vk::Device device, vk::SwapchainKHR swapchain, vk::Image image = nullptr, vk::Format format = vk::Format::eR8G8B8A8Unorm,
		vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor);
	static vk::UniqueShaderModule createShaderModule(vk::Device device, const char* pFilePath);
	static uint32_t selectQueueFamilyIndex(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);
	static vk::Format selectColorFormat(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);
	static vk::Format selectDepthStencilFormat(vk::PhysicalDevice physicalDevice);
	static vk::PresentModeKHR selectPresentMode(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, bool vsync = true, bool tripleBuffering = false, 
		bool tearing = false);
	static vk::SampleCountFlagBits getMaxUsableSampleCount(vk::PhysicalDevice physicalDevice);
	static uint32_t getMemoryTypeIndex(vk::PhysicalDevice physicalDevice, uint32_t typeBits, vk::MemoryPropertyFlags properties);
};