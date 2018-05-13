#pragma once

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

enum class TextureFiltering {
	Default,
	Bilinear,
	Trilinear,
	Aniso2x,
	Aniso4x,
	Aniso8x,
	Aniso16x,
};

struct GpuInfo {
	uint8_t id;
	const char* name;
	uint32_t memory;
	bool physical;
};

class Renderer {
public:
	/// This function initializes the renderer class for the given GLFW window.
	/// This must be the first function called after creating the object. 
	/// This function call will fail if:
	/// - The GLFWwindow handle is invalid.
	/// - A Vulkan loader was not found.
	/// - The required Vulkan instance extensions or layers were not found.
	/// - Vulkan Instance creation failed.
	/// - Vulkan Surface creation failed.
	/// - Vulkan did not find any physical devices.
	/// Return: This call will return the status of whether the 
	/// renderer was successfully created.
	bool createRendererForWindow(GLFWwindow* window);

	/// Returns a list of all gpus along with information about each one of them.
	/// id - The identifier of this GPU.
	/// name - The name of this GPU.
	/// memory - The maximum amount of video memory for this GPU.
	/// physical - A boolean of whether this GPU is a physical graphics card. 
	/// If this is false it could be software emulated, onboard, etc and is not the preferred option.
	std::vector<GpuInfo> enumerateGpus();

	/// Description: Selects the input GPU for rendering and surface operations.
	/// This function will fail if:
	/// - Hardware cannot present to the surface.
	/// - Hardware does not support graphics or surface operations.
	/// - Vulkan Device creation failed.
	/// - Vulkan Swapchain creation failed.
	/// Returns: This call will return the status of whether the input 
	/// physical device was selected successfully.
	/// Notes: If a previous physicalDevice was selected and this function fails,
	/// it does not preserve state.
	bool selectGpu(uint8_t id);

	void setVsync(bool state);
	bool getVsync();
	void setTextureFiltering(TextureFiltering textureFiltering);
	TextureFiltering getTextureFiltering();
private:
	vk::UniqueInstance instance;
	vk::UniqueSurfaceKHR surface;
	std::vector<vk::PhysicalDevice> physicalDevices;
	vk::UniqueDevice device;
	vk::UniqueSwapchainKHR swapchain;

	bool vsync;
	TextureFiltering textureFilter;
	bool deviceAnisotropy;
	float deviceMaxAnisotropy;
};
