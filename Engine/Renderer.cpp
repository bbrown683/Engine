#include "Renderer.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

Renderer::Renderer(GLFWwindow* window, bool trace) {
	std::vector<vk::ExtensionProperties> extensionProperties = vk::enumerateInstanceExtensionProperties();
	for (vk::ExtensionProperties property : extensionProperties)
		availableExtensions.push_back(property.extensionName);
	std::vector<vk::LayerProperties> layerProperties = vk::enumerateInstanceLayerProperties();
	for (vk::LayerProperties property : layerProperties)
		availableLayers.push_back(property.layerName);

	vk::ApplicationInfo appInfo;
#ifdef VK_VERSION_1_1
	appInfo.setApiVersion(VK_API_VERSION_1_1);
#else
	appInfo.setApiVersion(VK_API_VERSION_1_0);
#endif
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

	instance = vk::createInstanceUnique(instanceInfo);

#if defined(__linux__)
	// Detect MIR, Xcb, Wayland.
#elif defined(_WIN32)
	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.setHinstance(GetModuleHandle(nullptr));
	surfaceCreateInfo.setHwnd(glfwGetWin32Window(window));
	surface = instance->createWin32SurfaceKHRUnique(surfaceCreateInfo);
#endif

	physicalDevices = instance->enumeratePhysicalDevices();
	for (vk::PhysicalDevice physicalDevice : physicalDevices) {
		physicalDeviceFeatures.push_back(physicalDevice.getFeatures());
		physicalDeviceQueueFamilies.push_back(physicalDevice.getQueueFamilyProperties());
		physicalDeviceProperties.push_back(physicalDevice.getProperties());
		physicalDeviceSurfaceCapabilities.push_back(physicalDevice.getSurfaceCapabilitiesKHR(surface.get()));
		physicalDeviceSurfaceFormats.push_back(physicalDevice.getSurfaceFormatsKHR(surface.get()));
		physicalDeviceSurfacePresentModes.push_back(physicalDevice.getSurfacePresentModesKHR(surface.get()));
	}
}

vk::PhysicalDevice& Renderer::selectPhysicalDevice(std::vector<vk::PhysicalDevice>& physicalDevices) {
	return physicalDevices[0];
}

vk::Format Renderer::selectSurfaceFormat(std::vector<vk::SurfaceFormatKHR> surfaceFormats) {
	return vk::Format::eR8G8B8A8Unorm;
}
