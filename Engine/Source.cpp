#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <libconfig.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_TYPESAFE_CONVERSION
#include <vulkan/vulkan.hpp>

#include <iostream>

#include "GameConfig.hpp"
#include "AudioManager.hpp"
#include "GraphicsManager.hpp"
#include "WindowManager.hpp"

enum class ErrorSource {
	Glfw = -1,
	Vulkan = -2
};

int main(int argc, char** argv) {
	std::vector<const char*> args;
	args.insert(args.begin(), argv, argv + argc);

	// CONFIG
	// ============================================================================================
	config_t config;
	config_init(&config);
	
	// Check if there is a config file already.
	// If not create a default one.
	if (!config_read_file(&config, "settings.cfg")) {
		config_set_tab_width(&config, 4);
		config_setting_t* rootGroup = config_root_setting(&config);
		config_setting_t* version = config_setting_add(rootGroup, "version", CONFIG_TYPE_ARRAY);
		config_setting_set_int_elem(version, -1, 1);
		config_setting_set_int_elem(version, -1, 0);
		config_setting_set_int_elem(version, -1, 0);
		config_setting_t* applicationGroup = config_setting_add(rootGroup, "application", CONFIG_TYPE_GROUP);
		config_setting_t* windowGroup = config_setting_add(applicationGroup, "window", CONFIG_TYPE_GROUP);
		config_setting_t* graphicsGroup = config_setting_add(applicationGroup, "graphics", CONFIG_TYPE_GROUP);
		config_setting_t* audioGroup = config_setting_add(applicationGroup, "audio", CONFIG_TYPE_GROUP);

		config_setting_t* x = config_setting_add(windowGroup, "x", CONFIG_TYPE_INT);
		config_setting_set_int(x, 0);
		config_setting_t* y = config_setting_add(windowGroup, "y", CONFIG_TYPE_INT);
		config_setting_set_int(y, 0);
		config_setting_t* width = config_setting_add(windowGroup, "width", CONFIG_TYPE_INT);
		config_setting_set_int(width, 1024);
		config_setting_t* height = config_setting_add(windowGroup, "height", CONFIG_TYPE_INT);
		config_setting_set_int(height, 768);
		config_setting_t* mode = config_setting_add(windowGroup, "mode", CONFIG_TYPE_INT);
		config_setting_set_int(mode, static_cast<int>(WindowMode::Windowed));
		config_setting_t* vsync = config_setting_add(graphicsGroup, "vsync", CONFIG_TYPE_BOOL);
		config_setting_set_bool(vsync, true);
		config_setting_t* textureQuality = config_setting_add(graphicsGroup, "textureQuality", CONFIG_TYPE_INT);
		config_setting_set_int(textureQuality, static_cast<int>(Quality::Ultra));
		config_setting_t* textureFilterQuality = config_setting_add(graphicsGroup, "textureFiltering", CONFIG_TYPE_INT);
		config_setting_set_int(textureFilterQuality, static_cast<int>(TextureFiltering::Anisotropic16x));
		config_setting_t* masterVolume = config_setting_add(audioGroup, "masterVolume", CONFIG_TYPE_FLOAT);
		config_setting_set_float(masterVolume, 1.0f);
		config_write_file(&config, "settings.cfg");
	}
	config_destroy(&config);
	// ===============================================================================================

	if (!glfwInit())
		return static_cast<int>(ErrorSource::Glfw);

	if (!glfwVulkanSupported())
		return static_cast<int>(ErrorSource::Glfw);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(1024, 768, "Engine", nullptr, nullptr);

	std::vector<vk::ExtensionProperties> extensionProperties = vk::enumerateInstanceExtensionProperties();
	bool surfaceKHRSupport = false, surfaceKHRWin32Support = false;
	for (vk::ExtensionProperties extension : extensionProperties) {
		if (strcmp(extension.extensionName, VK_KHR_SURFACE_EXTENSION_NAME))
			surfaceKHRSupport = true;
		if (strcmp(extension.extensionName, VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
			surfaceKHRWin32Support = true;
	}
	
	if (!surfaceKHRSupport || !surfaceKHRWin32Support)
		return static_cast<int>(ErrorSource::Vulkan);

	std::vector<vk::LayerProperties> layerProperties = vk::enumerateInstanceLayerProperties();

	vk::ApplicationInfo appInfo;
	appInfo.setApiVersion(VK_MAKE_VERSION(1, 0, 0));
	appInfo.setPApplicationName("Engine");

	std::vector<const char*> instanceExtensions;
	std::vector<const char*> instanceLayers;

#ifdef _DEBUG
	instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	instanceLayers.push_back("VK_LAYER_LUNARG_api_dump");
	instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

	uint32_t glfwExtensionCount;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	instanceExtensions.insert(instanceExtensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

	vk::InstanceCreateInfo instanceInfo;
	instanceInfo.setPApplicationInfo(&appInfo);
	instanceInfo.setEnabledExtensionCount(static_cast<uint32_t>(instanceExtensions.size()));
	instanceInfo.setPpEnabledExtensionNames(instanceExtensions.data());
	instanceInfo.setEnabledLayerCount(static_cast<uint32_t>(instanceLayers.size()));
	instanceInfo.setPpEnabledLayerNames(instanceLayers.data());

	vk::UniqueInstance instance;
	try {
		instance = vk::createInstanceUnique(instanceInfo);
	}
	catch (std::runtime_error e) {
		return static_cast<int>(ErrorSource::Vulkan);
	}

	vk::UniqueSurfaceKHR surface;
	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.setHinstance(GetModuleHandle(nullptr));
	surfaceCreateInfo.setHwnd(glfwGetWin32Window(window));
	surface = instance->createWin32SurfaceKHRUnique(surfaceCreateInfo);

	// TODO: Allow user to pick GPU via settings.
	std::vector<vk::PhysicalDevice> physicalDevices = instance->enumeratePhysicalDevices();
	vk::PhysicalDevice physicalDevice = physicalDevices.front();

	std::vector<vk::ExtensionProperties> deviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
	
	// Check for VK_KHR_swapchain extension.
	bool swapchainSupport = false;
	for (vk::ExtensionProperties deviceExtension : deviceExtensions)
		if (std::strcmp(deviceExtension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
			swapchainSupport = true;

	if (!swapchainSupport)
		return static_cast<int>(ErrorSource::Vulkan);

	//vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();

	std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
	vk::SurfaceCapabilitiesKHR surfaceCapabilities;
	std::vector<vk::SurfaceFormatKHR> surfaceFormats;
	std::vector<vk::PresentModeKHR> surfacePresentModes;

	std::vector<uint32_t> graphicsSupport;
	std::vector<uint32_t> surfaceSupport;

	for (int i = 0; i < queueFamilies.size(); i++) {
		// Only graphics queues should be checked.
		if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
			graphicsSupport.push_back(i);

		// Check for surface support.
		VkBool32 supportsSurface = physicalDevice.getSurfaceSupportKHR(i, surface.get());
		if (supportsSurface) {
			surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface.get());
			surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface.get());
			surfacePresentModes = physicalDevice.getSurfacePresentModesKHR(surface.get());
			surfaceSupport.push_back(i);
		}
	}

	// Graphics or Surface are not supported.
	if (graphicsSupport.empty() || surfaceSupport.empty())
		return static_cast<int>(ErrorSource::Vulkan);

	float priority = 1.0f;

	vk::DeviceQueueCreateInfo deviceQueueInfos;
	deviceQueueInfos.setQueueFamilyIndex(0);
	deviceQueueInfos.setQueueCount(1);
	deviceQueueInfos.setPQueuePriorities(&priority);

	std::vector<const char*> enabledDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	vk::PhysicalDeviceFeatures enabledFeatures;
	enabledFeatures.setSamplerAnisotropy(true);

	vk::DeviceCreateInfo deviceInfo;
	deviceInfo.setEnabledLayerCount(0);
	deviceInfo.setPpEnabledExtensionNames(enabledDeviceExtensions.data());
	deviceInfo.setEnabledExtensionCount(static_cast<uint32_t>(enabledDeviceExtensions.size()));
	deviceInfo.setPQueueCreateInfos(&deviceQueueInfos);
	deviceInfo.setQueueCreateInfoCount(1);
	deviceInfo.setPEnabledFeatures(&enabledFeatures);

	vk::UniqueDevice device;	
	try {
		device = physicalDevice.createDeviceUnique(deviceInfo);
	}
	catch (std::runtime_error e) {
		return static_cast<int>(ErrorSource::Vulkan);
	}

	// Select present mode.
	// Prefer to use Mailbox present mode if it exists.
	vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
	if (surfaceCapabilities.minImageCount > 2)
		for (vk::PresentModeKHR tempPresentMode : surfacePresentModes)
			if (tempPresentMode == vk::PresentModeKHR::eMailbox)
				presentMode = tempPresentMode;

	// Select swapchain format.
	vk::Format format = vk::Format::eUndefined;

	// Check to see if the driver lets us select which one we want.
	if (surfaceFormats.size() == 1 && surfaceFormats[0].format == vk::Format::eUndefined)
		format = vk::Format::eR8G8B8A8Unorm;
	else {
		// Iterate through each format and check to see if it has the format we want.
		for (vk::SurfaceFormatKHR surfaceFormat : surfaceFormats)
			if (surfaceFormat.format == vk::Format::eR8G8B8A8Unorm)
				format = surfaceFormat.format;
		// If we still didnt find a format just pick the first one we get.
		if (format == vk::Format::eUndefined)
			format = surfaceFormats.front().format;
	}

	vk::SwapchainCreateInfoKHR swapchainInfo;
	swapchainInfo.setSurface(surface.get());
	swapchainInfo.setMinImageCount(surfaceCapabilities.minImageCount);
	swapchainInfo.setImageExtent(surfaceCapabilities.currentExtent);
	swapchainInfo.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear);
	swapchainInfo.setImageFormat(format);
	swapchainInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst);
	swapchainInfo.setClipped(true);
	swapchainInfo.setImageArrayLayers(1);
	swapchainInfo.setPresentMode(presentMode);
	swapchainInfo.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity);
	swapchainInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);

	vk::UniqueSwapchainKHR swapchain = device->createSwapchainKHRUnique(swapchainInfo);

	vk::CommandPoolCreateInfo commandPoolInfo;
	commandPoolInfo.setQueueFamilyIndex(0);
	vk::UniqueCommandPool commandPool = device->createCommandPoolUnique(commandPoolInfo);
	
	vk::CommandBufferAllocateInfo allocateInfo;
	allocateInfo.setCommandPool(commandPool.get());
	allocateInfo.setCommandBufferCount(1);
	allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);

	auto commandBuffer = device->allocateCommandBuffersUnique(allocateInfo);

	vk::FenceCreateInfo fenceInfo;
	vk::UniqueFence fence = device->createFenceUnique(fenceInfo);

	vk::Viewport viewport;
	viewport.setWidth(static_cast<float>(surfaceCapabilities.currentExtent.width));
	viewport.setHeight(static_cast<float>(surfaceCapabilities.currentExtent.height));
	viewport.setMinDepth(0.0f);
	viewport.setMaxDepth(1.0f);

	vk::CommandBufferBeginInfo beginInfo;
	commandBuffer[0]->begin(beginInfo);
	commandBuffer[0]->setViewport(0, viewport);
	commandBuffer[0]->end();

	vk::Queue queue = device->getQueue(0, 0);
	vk::SubmitInfo submitInfo;
	submitInfo.setCommandBufferCount(1);
	submitInfo.setPCommandBuffers(&commandBuffer[0].get());
	queue.submit(submitInfo, fence.get());

	// Wait for command buffer to complete.
	vk::Result fenceStatus = device->waitForFences(fence.get(), true, UINT64_MAX);
	if (fenceStatus != vk::Result::eSuccess && fenceStatus != vk::Result::eTimeout)
		return static_cast<int>(ErrorSource::Vulkan);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}