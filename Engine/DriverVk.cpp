/*
MIT License

Copyright (c) 2018 Ben Brown

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "DriverVk.hpp"

#include <iostream>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

DriverVk::DriverVk(GLFWwindow* pWindow) : IDriver(pWindow) {}

bool DriverVk::initialize() {
	// GLFW can tell us if there is a vulkan loader.
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
		m_pInstance.swap(instanceResult.value);
	else {
		std::cerr << "CRITICAL: A Vulkan driver was not detected!\n";
		return false;
	}

	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
	surfaceCreateInfo.hwnd = glfwGetWin32Window(getWindow());

	auto surfaceResult = m_pInstance->createWin32SurfaceKHRUnique(surfaceCreateInfo);
	if (surfaceResult.result == vk::Result::eSuccess)
		m_pSurface.swap(surfaceResult.value);
	else {
		std::cerr << "CRITICAL: Could not create a Vulkan rendering surface!\n";
		return false;
	}

	auto m_PhysicalDevicesResult = m_pInstance->enumeratePhysicalDevices();
	if (m_PhysicalDevicesResult.result == vk::Result::eSuccess)
		m_PhysicalDevices.swap(m_PhysicalDevicesResult.value);
	else {
		std::cerr << "CRITICAL: Could not detect a Vulkan supported hardware device!\n";
		return false;
	}
	return true;
}

std::vector<Gpu> DriverVk::getGpus() {
	std::vector<Gpu> info;
	uint8_t counter = 0;
	for (vk::PhysicalDevice physicalDevice : m_PhysicalDevices) {
		vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
		bool physical = properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu ? true : false;
		info.push_back(Gpu({
			counter++,
			properties.deviceName,
			0,
			physical
			}
		));
	}
	return info;
}

bool DriverVk::selectGpu(uint8_t id) {
	std::vector<vk::ExtensionProperties> deviceExtensions;
	auto deviceExtensionsResult = m_PhysicalDevices[id].enumerateDeviceExtensionProperties();
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

	vk::PhysicalDeviceFeatures features = m_PhysicalDevices[id].getFeatures();
	if (features.samplerAnisotropy) {
		anisotropy = true;
		vk::PhysicalDeviceProperties properties = m_PhysicalDevices[id].getProperties();
		maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	}

	std::vector<vk::QueueFamilyProperties> queueFamilies = m_PhysicalDevices[id].getQueueFamilyProperties();
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
		auto surfaceSupportResult = m_PhysicalDevices[id].getSurfaceSupportKHR(i, m_pSurface.get());
		if (surfaceSupportResult.result == vk::Result::eSuccess)
			supportsSurface = surfaceSupportResult.value;
		if (supportsSurface) {
			auto surfaceCapabilitiesResult = m_PhysicalDevices[id].getSurfaceCapabilitiesKHR(m_pSurface.get());
			if (surfaceCapabilitiesResult.result == vk::Result::eSuccess)
				surfaceCapabilities = surfaceCapabilitiesResult.value;

			auto surfaceFormatsResult = m_PhysicalDevices[id].getSurfaceFormatsKHR(m_pSurface.get());
			if (surfaceFormatsResult.result == vk::Result::eSuccess)
				surfaceFormats = surfaceFormatsResult.value;

			auto surfacePresentModesResult = m_PhysicalDevices[id].getSurfacePresentModesKHR(m_pSurface.get());
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

	auto deviceResult = m_PhysicalDevices[id].createDeviceUnique(deviceInfo);
	if (deviceResult.result == vk::Result::eSuccess) {
		// TODO: Reset device if its already been initialized previously.
		m_pDevice.swap(deviceResult.value);
	}
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
	swapchainInfo.surface = m_pSurface.get();

	auto swapchainResult = m_pDevice->createSwapchainKHRUnique(swapchainInfo);
	if (swapchainResult.result == vk::Result::eSuccess) {
		// TODO: Reset swapchain if its already been initialized previously.
		m_pSwapchain.swap(swapchainResult.value);
	}
	else {
		throw std::runtime_error("CRITICAL: Failed to create a swapchain for rendering surface!");
		return false;
	}
	return true;
}
