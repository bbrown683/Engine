#include "GraphicsManager.hpp"

GraphicsManager::GraphicsManager(UserGraphicsSettings settings, GLFWwindow* window) : settings(settings) {
	if (!glfwVulkanSupported())
		throw std::runtime_error("CRITICAL: Vulkan loader was not found!");

	std::vector<vk::ExtensionProperties> extensionProperties = vk::enumerateInstanceExtensionProperties();
	bool surfaceKHRSupport = false, surfaceKHRWin32Support = false;
	for (vk::ExtensionProperties extension : extensionProperties) {
		if (strcmp(extension.extensionName, VK_KHR_SURFACE_EXTENSION_NAME))
			surfaceKHRSupport = true;
		if (strcmp(extension.extensionName, VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
			surfaceKHRWin32Support = true;
	}

	if (!surfaceKHRSupport || !surfaceKHRWin32Support)
		throw std::runtime_error("CRITICAL: Vulkan driver does not support rendering to a surface!");

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

	try {
		instance = vk::createInstanceUnique(instanceInfo);
	}
	catch (std::runtime_error e) {
		throw std::runtime_error("CRITICAL: Driver support for Vulkan was not detected!");
	}


	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.setHinstance(GetModuleHandle(nullptr));
	surfaceCreateInfo.setHwnd(glfwGetWin32Window(window));
	surface = instance->createWin32SurfaceKHRUnique(surfaceCreateInfo);

	try {
		physicalDevices = instance->enumeratePhysicalDevices();
	}
	catch (std::runtime_error e) {
		throw std::runtime_error("CRITICAL: Could not detect a Vulkan supported hardware device!");
	}
}

std::vector<Renderer> GraphicsManager::getRenderers() {
	return std::vector<Renderer>();
}
