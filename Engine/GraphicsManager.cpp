#include "GraphicsManager.hpp"

GraphicsManager::GraphicsManager(UserGraphicsSettings settings, GLFWwindow* window) : settings(settings) {
	if (!glfwVulkanSupported())
		throw std::runtime_error("CRITICAL: Vulkan loader was not found!");

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
		throw std::runtime_error("CRITICAL: Vulkan driver does not support rendering to a surface!");

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
	instanceLayers.push_back("VK_LAYER_LUNARG_api_dump");
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

	auto instanceResult = vk::createInstance(instanceInfo);
	if (instanceResult.result == vk::Result::eSuccess)
		instance = instanceResult.value;
	else
		throw std::runtime_error("CRITICAL: A Vulkan driver was not detected!");

	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
	surfaceCreateInfo.hwnd = glfwGetWin32Window(window);
	auto surfaceResult = instance.createWin32SurfaceKHR(surfaceCreateInfo);
	if (surfaceResult.result == vk::Result::eSuccess)
		surface = surfaceResult.value;
	else
		throw std::runtime_error("CRITICAL: Could not create a Vulkan rendering surface!");

	auto physicalDevicesResult = instance.enumeratePhysicalDevices();
	if (physicalDevicesResult.result == vk::Result::eSuccess)
		physicalDevices = physicalDevicesResult.value;
	else
		throw std::runtime_error("CRITICAL: Could not detect a Vulkan supported hardware device!");
}

GraphicsManager::~GraphicsManager() {
	instance.destroySurfaceKHR(surface);
	instance.destroy();
}

std::vector<Renderer> GraphicsManager::getRenderers() {
	return std::vector<Renderer>();
}
