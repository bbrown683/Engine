#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_TYPESAFE_CONVERSION
#include <vulkan/vulkan.hpp>

#include <iostream>

enum class ErrorSource {
	Glfw = -1,
	Vulkan = -2
};

int main(int argc, char** argv) {
	std::vector<const char*> args;
	args.insert(args.begin(), argv, argv + argc);

	if (!glfwInit())
		return static_cast<int>(ErrorSource::Glfw);

	if (!glfwVulkanSupported())
		return static_cast<int>(ErrorSource::Glfw);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(1024, 768, "Engine", nullptr, nullptr);

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
	
	if (!surfaceKHRSupport || !surfaceKHRWin32Support)
		return static_cast<int>(ErrorSource::Vulkan);

	std::vector<vk::LayerProperties> layerProperties;
	auto layerPropertiesResult = vk::enumerateInstanceLayerProperties();
	if (layerPropertiesResult.result == vk::Result::eSuccess)
		layerProperties = layerPropertiesResult.value;
	else
		return static_cast<int>(ErrorSource::Vulkan);

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
	auto instanceResult = vk::createInstanceUnique(instanceInfo);
	if (instanceResult.result == vk::Result::eSuccess)
		instance.swap(instanceResult.value);
	else
		return static_cast<int>(ErrorSource::Vulkan);

	vk::UniqueSurfaceKHR surface;
	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.setHinstance(GetModuleHandle(nullptr));
	surfaceCreateInfo.setHwnd(glfwGetWin32Window(window));
	auto surfaceResult = instance->createWin32SurfaceKHRUnique(surfaceCreateInfo);
	if (surfaceResult.result == vk::Result::eSuccess)
		surface.swap(surfaceResult.value);
	else
		return static_cast<int>(ErrorSource::Vulkan);

	// TODO: Allow user to pick GPU via settings.
	std::vector<vk::PhysicalDevice> physicalDevices;
	auto physicalDevicesResult = instance->enumeratePhysicalDevices();
	if (physicalDevicesResult.result == vk::Result::eSuccess)
		physicalDevices.swap(physicalDevicesResult.value);
	else
		return static_cast<int>(ErrorSource::Vulkan);

	vk::PhysicalDevice physicalDevice = physicalDevices.front();
	std::vector<vk::ExtensionProperties> deviceExtensions;
	auto deviceExtensionsResult = physicalDevice.enumerateDeviceExtensionProperties();
	if (deviceExtensionsResult.result == vk::Result::eSuccess)
		deviceExtensions = deviceExtensionsResult.value;
	else
		return static_cast<int>(ErrorSource::Vulkan);

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
		VkBool32 supportsSurface = VK_TRUE;
		auto surfaceSupportResult = physicalDevice.getSurfaceSupportKHR(i, surface.get());
		if (surfaceSupportResult.result == vk::Result::eSuccess)
			supportsSurface = surfaceSupportResult.value;
		if (supportsSurface) {
			auto surfaceCapabilitiesResult = physicalDevice.getSurfaceCapabilitiesKHR(surface.get());
			if (surfaceCapabilitiesResult.result == vk::Result::eSuccess)
				surfaceCapabilities = surfaceCapabilitiesResult.value;
			else 
				return static_cast<int>(ErrorSource::Vulkan);

			auto surfaceFormatsResult = physicalDevice.getSurfaceFormatsKHR(surface.get());
			if (surfaceFormatsResult.result == vk::Result::eSuccess)
				surfaceFormats = surfaceFormatsResult.value;
			else
				return static_cast<int>(ErrorSource::Vulkan);

			auto surfacePresentModesResult = physicalDevice.getSurfacePresentModesKHR(surface.get());
			if (surfacePresentModesResult.result == vk::Result::eSuccess)
				surfacePresentModes = surfacePresentModesResult.value;
			else
				return static_cast<int>(ErrorSource::Vulkan);

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
	deviceInfo.pEnabledFeatures = &enabledFeatures;
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();
	deviceInfo.enabledExtensionCount = static_cast<uint32_t>(enabledDeviceExtensions.size());
	deviceInfo.pQueueCreateInfos = &deviceQueueInfos;
	deviceInfo.queueCreateInfoCount = 1;

	vk::UniqueDevice device;	
	auto deviceResult = physicalDevice.createDeviceUnique(deviceInfo);
	if (deviceResult.result == vk::Result::eSuccess)
		device.swap(deviceResult.value);
	else
		return static_cast<int>(ErrorSource::Vulkan);

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

	vk::UniqueSwapchainKHR swapchain;
	auto swapchainResult = device->createSwapchainKHRUnique(swapchainInfo);
	if (swapchainResult.result == vk::Result::eSuccess)
		swapchain.swap(swapchainResult.value);
	else
		return static_cast<int>(ErrorSource::Vulkan);

	vk::CommandPoolCreateInfo commandPoolInfo;
	commandPoolInfo.queueFamilyIndex = 0;

	vk::UniqueCommandPool commandPool;
	auto commandPoolResult = device->createCommandPoolUnique(commandPoolInfo);
	if (commandPoolResult.result == vk::Result::eSuccess)
		commandPool.swap(commandPoolResult.value);
	else
		return static_cast<int>(ErrorSource::Vulkan);

	vk::CommandBufferAllocateInfo allocateInfo;
	allocateInfo.commandPool = commandPool.get();
	allocateInfo.commandBufferCount = 1;
	allocateInfo.level = vk::CommandBufferLevel::ePrimary;

	std::vector<vk::UniqueCommandBuffer> commandBuffers;
	auto commandBuffersResult = device->allocateCommandBuffersUnique(allocateInfo);
	if (commandBuffersResult.result == vk::Result::eSuccess) {
		commandBuffers.swap(commandBuffersResult.value);
	}
	else
		return static_cast<int>(ErrorSource::Vulkan);

	vk::FenceCreateInfo fenceInfo;
	vk::UniqueFence fence;
	auto fenceResult = device->createFenceUnique(fenceInfo);
	if (fenceResult.result == vk::Result::eSuccess)
		fence.swap(fenceResult.value);
	else
		return static_cast<int>(ErrorSource::Vulkan);

	vk::Viewport viewport;
	viewport.width = static_cast<float>(surfaceCapabilities.currentExtent.width);
	viewport.height = static_cast<float>(surfaceCapabilities.currentExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::CommandBufferBeginInfo beginInfo;
	commandBuffers[0]->begin(beginInfo);
	commandBuffers[0]->setViewport(0, viewport);
	commandBuffers[0]->end();

	vk::Queue queue = device->getQueue(0, 0);
	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[0].get();
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