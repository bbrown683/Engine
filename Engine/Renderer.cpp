#include "Renderer.hpp"

#include <iostream>

bool Renderer::createRendererForWindow(GLFWwindow* window) {
	if (!window) {
		std::cerr << "CRITICAL: GLFW window is not a valid pointer!\n";
		return false;
	}

	if (!glfwVulkanSupported()) {
		std::cerr << "CRITICAL: Vulkan loader was not found!\n";
		return false;
	}

	std::vector<vk::ExtensionProperties> extensionProperties;
	auto extensionPropertiesResult = vk::enumerateInstanceExtensionProperties();
	if (extensionPropertiesResult.result == vk::Result::eSuccess)
		extensionProperties = extensionPropertiesResult.value;
	bool surfaceKHRSupport = false, surfaceKHRWin32Support = false;
	for (vk::ExtensionProperties extension : extensionProperties) {
		if (strcmp(extension.extensionName, VK_KHR_SURFACE_EXTENSION_NAME))
			surfaceKHRSupport = true;
		if (strcmp(extension.extensionName, VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
			surfaceKHRWin32Support = true;
	}

	if (!surfaceKHRSupport || !surfaceKHRWin32Support) {
		std::cerr << "CRITICAL: Vulkan driver does not support rendering to a surface!\n";
		return false;
	}

	// We do not need to throw an exception if we cannot query layers.
	std::vector<vk::LayerProperties> layerProperties;
	auto layerPropertiesResult = vk::enumerateInstanceLayerProperties();
	if (layerPropertiesResult.result == vk::Result::eSuccess)
		layerProperties = layerPropertiesResult.value;

	vk::ApplicationInfo appInfo;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	std::vector<const char*> instanceExtensions;
	std::vector<const char*> instanceLayers;

#ifdef _DEBUG
	instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
//	instanceLayers.push_back("VK_LAYER_LUNARG_api_dump");
	instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

	uint32_t glfwExtensionCount;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	instanceExtensions.insert(instanceExtensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

	vk::InstanceCreateInfo instanceInfo;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();
	instanceInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
	instanceInfo.ppEnabledLayerNames = instanceLayers.data();

	auto instanceResult = vk::createInstanceUnique(instanceInfo);
	if (instanceResult.result == vk::Result::eSuccess)
		instance = std::move(instanceResult.value);
	else {
		std::cerr << "CRITICAL: A Vulkan driver was not detected!\n";
		return false;
	}

	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
	surfaceCreateInfo.hwnd = glfwGetWin32Window(window);

	auto surfaceResult = instance->createWin32SurfaceKHRUnique(surfaceCreateInfo);
	if (surfaceResult.result == vk::Result::eSuccess)
		surface.swap(surfaceResult.value);
	else {
		std::cerr << "CRITICAL: Could not create a Vulkan rendering surface!\n";
		return false;
	}

	auto physicalDevicesResult = instance->enumeratePhysicalDevices();
	if (physicalDevicesResult.result == vk::Result::eSuccess)
		physicalDevices.swap(physicalDevicesResult.value);
	else {
		std::cerr << "CRITICAL: Could not detect a Vulkan supported hardware device!\n";
		return false;
	}
	return true;
}

std::vector<GpuInfo> Renderer::enumerateGpus() {
	std::vector<GpuInfo> info;
	uint8_t counter = 0;
	for (vk::PhysicalDevice physicalDevice : physicalDevices) {
		vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
		bool physical = properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu ? true : false;
		info.push_back(GpuInfo({
			counter++,
			properties.deviceName,
			0,
			physical
		}));
	}
	return info;
}

bool Renderer::selectGpu(uint8_t id) {
	std::vector<vk::ExtensionProperties> deviceExtensions;
	auto deviceExtensionsResult = physicalDevices[id].enumerateDeviceExtensionProperties();
	if (deviceExtensionsResult.result == vk::Result::eSuccess)
		deviceExtensions = deviceExtensionsResult.value;

	// Check for VK_KHR_swapchain extension.
	bool swapchainSupport = false;
	for (vk::ExtensionProperties deviceExtension : deviceExtensions)
		if (std::strcmp(deviceExtension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
			swapchainSupport = true;

	if (!swapchainSupport) {
		std::cerr << "CRITICAL: Hardware device does not support presenting to a surface!\n";
		return false;
	}

	vk::PhysicalDeviceFeatures features = physicalDevices[id].getFeatures();
	if (features.samplerAnisotropy) {
		deviceAnisotropy = true;
		vk::PhysicalDeviceProperties properties = physicalDevices[id].getProperties();
		deviceMaxAnisotropy = properties.limits.maxSamplerAnisotropy;
	}

	std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevices[id].getQueueFamilyProperties();
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
		VkBool32 supportsSurface = VK_TRUE;
		auto surfaceSupportResult = physicalDevices[id].getSurfaceSupportKHR(i, surface.get());
		if (surfaceSupportResult.result == vk::Result::eSuccess)
			supportsSurface = surfaceSupportResult.value;
		if (supportsSurface) {
			auto surfaceCapabilitiesResult = physicalDevices[id].getSurfaceCapabilitiesKHR(surface.get());
			if (surfaceCapabilitiesResult.result == vk::Result::eSuccess)
				surfaceCapabilities = surfaceCapabilitiesResult.value;

			auto surfaceFormatsResult = physicalDevices[id].getSurfaceFormatsKHR(surface.get());
			if (surfaceFormatsResult.result == vk::Result::eSuccess)
				surfaceFormats = surfaceFormatsResult.value;

			auto surfacePresentModesResult = physicalDevices[id].getSurfacePresentModesKHR(surface.get());
			if (surfacePresentModesResult.result == vk::Result::eSuccess)
				surfacePresentModes = surfacePresentModesResult.value;

			surfaceSupport.push_back(i);
		}
	}

	// Graphics or Surface are not supported.
	if (graphicsSupport.empty() || surfaceSupport.empty()) {
		std::cerr << "CRITICAL: Hardware device does not support drawing or surface operations!\n";
		return false;
	}

	float priority = 1.0f;

	vk::DeviceQueueCreateInfo deviceQueueInfos;
	deviceQueueInfos.queueCount = 1;
	deviceQueueInfos.queueFamilyIndex = 0;
	deviceQueueInfos.pQueuePriorities = &priority;

	std::vector<const char*> enabledDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	vk::PhysicalDeviceFeatures enabledFeatures;
	enabledFeatures.samplerAnisotropy = VK_TRUE;

	vk::DeviceCreateInfo deviceInfo;
	deviceInfo.pEnabledFeatures = &enabledFeatures;
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();
	deviceInfo.enabledExtensionCount = static_cast<uint32_t>(enabledDeviceExtensions.size());
	deviceInfo.pQueueCreateInfos = &deviceQueueInfos;
	deviceInfo.queueCreateInfoCount = 1;

	auto deviceResult = physicalDevices[id].createDeviceUnique(deviceInfo);
	if (deviceResult.result == vk::Result::eSuccess)
		device.swap(deviceResult.value);
	else {
		std::cerr << "CRITICAL: Failed to create a rendering device!\n";
		return false;
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
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapchainInfo.minImageCount = surfaceCapabilities.minImageCount;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageExtent = surfaceCapabilities.currentExtent;
	swapchainInfo.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
	swapchainInfo.imageFormat = format;
	swapchainInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
	swapchainInfo.presentMode = presentMode;
	swapchainInfo.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	swapchainInfo.surface = surface.get();

	auto swapchainResult = device->createSwapchainKHRUnique(swapchainInfo);
	if (swapchainResult.result == vk::Result::eSuccess)
		swapchain = std::move(swapchainResult.value);
	else {
		throw std::runtime_error("CRITICAL: Failed to create a swapchain for rendering surface!");
		return false;
	}
	return true;

}
