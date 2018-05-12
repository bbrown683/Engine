#include "Renderer.hpp"

#include <iostream>

Renderer::Renderer(UserGraphicsSettings settings, vk::PhysicalDevice& physicalDevice, vk::UniqueSurfaceKHR& surface) {
	std::vector<vk::ExtensionProperties> deviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
	// Check for VK_KHR_swapchain extension.
	bool swapchainSupport = false;
	for (vk::ExtensionProperties deviceExtension : deviceExtensions)
		if (std::strcmp(deviceExtension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
			swapchainSupport = true;

	if (!swapchainSupport) {
		std::cout << "CRITICAL: Hardware device does not support presenting to a surface!\n";
	}
	
	vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
	physicalDeviceName = properties.deviceName;

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
	if (graphicsSupport.empty() || surfaceSupport.empty()) {
		std::cout << "CRITICAL: Hardware device does not support drawing or surface operations!\n";
	}

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

	try {
		device = physicalDevice.createDeviceUnique(deviceInfo);
	}
	catch (std::runtime_error e) {
		std::cout << "CRITICAL: Failed to create a rendering device: " << e.what() << std::endl;
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

	try {
		swapchain = device->createSwapchainKHRUnique(swapchainInfo);
	}
	catch (std::runtime_error e) {
		std::cout << "CRITICAL: Failed to create a swapchain: " << e.what() << std::endl;
	}
}
