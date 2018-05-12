#include "Renderer.hpp"

#include <iostream>

Renderer::Renderer(UserGraphicsSettings settings, vk::PhysicalDevice& physicalDevice, vk::UniqueSurfaceKHR& surface) {
	std::vector<vk::ExtensionProperties> deviceExtensions;
	auto deviceExtensionsResult = physicalDevice.enumerateDeviceExtensionProperties();
	if (deviceExtensionsResult.result == vk::Result::eSuccess)
		deviceExtensions = deviceExtensionsResult.value;
	
	// Check for VK_KHR_swapchain extension.
	bool swapchainSupport = false;
	for (vk::ExtensionProperties deviceExtension : deviceExtensions)
		if (std::strcmp(deviceExtension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
			swapchainSupport = true;

	if (!swapchainSupport)
		throw std::runtime_error("CRITICAL: Hardware device does not support presenting to a surface!");

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
		VkBool32 supportsSurface = VK_TRUE;
		auto surfaceSupportResult = physicalDevice.getSurfaceSupportKHR(i, surface.get());
		if (surfaceSupportResult.result == vk::Result::eSuccess)
			supportsSurface = surfaceSupportResult.value;
		if (supportsSurface) {
			auto surfaceCapabilitiesResult = physicalDevice.getSurfaceCapabilitiesKHR(surface.get());
			if (surfaceCapabilitiesResult.result == vk::Result::eSuccess)
				surfaceCapabilities = surfaceCapabilitiesResult.value;

			auto surfaceFormatsResult = physicalDevice.getSurfaceFormatsKHR(surface.get());
			if (surfaceFormatsResult.result == vk::Result::eSuccess)
				surfaceFormats = surfaceFormatsResult.value;

			auto surfacePresentModesResult = physicalDevice.getSurfacePresentModesKHR(surface.get());
			if (surfacePresentModesResult.result == vk::Result::eSuccess)
				surfacePresentModes = surfacePresentModesResult.value;

			surfaceSupport.push_back(i);
		}
	}

	// Graphics or Surface are not supported.
	if (graphicsSupport.empty() || surfaceSupport.empty())
		throw std::runtime_error("CRITICAL: Hardware device does not support drawing or surface operations!");

	float priority = 1.0f;

	vk::DeviceQueueCreateInfo deviceQueueInfos;
	deviceQueueInfos.setQueueFamilyIndex(0);
	deviceQueueInfos.setQueueCount(1);
	deviceQueueInfos.setPQueuePriorities(&priority);

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

	auto deviceResult = physicalDevice.createDevice(deviceInfo);
	if (deviceResult.result == vk::Result::eSuccess)
		device = deviceResult.value;
	else
		throw std::runtime_error("CRITICAL: Failed to create a rendering device!");

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

	auto swapchainResult = device.createSwapchainKHR(swapchainInfo);
	if (swapchainResult.result == vk::Result::eSuccess)
		swapchain = swapchainResult.value;
	else
		throw std::runtime_error("CRITICAL: Failed to create a swapchain for rendering surface!");
}

Renderer::~Renderer() {
	device.destroySwapchainKHR(swapchain);
	device.destroy();
}
