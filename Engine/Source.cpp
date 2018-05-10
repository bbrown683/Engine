#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <libconfig.h>
#if defined(__linux__)

#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#define VULKAN_HPP_TYPESAFE_CONVERSION
#include <vulkan/vulkan.hpp>

#include "Renderer.hpp"

enum class ErrorType {
	GlfwInitFailed						= -1,
	GlfwVulkanNotFound					= -2,
	VulkanInstanceCreationFailed		= -3,
	VulkanSurfaceCreationFailed			= -4,
};

int main(int argc, char** argv) {
	std::vector<const char*> args;
	args.insert(args.begin(), argv, argv + argc);

	int initResult = glfwInit();
	if (initResult != GLFW_TRUE)
		return static_cast<int>(ErrorType::GlfwInitFailed);

	if (!glfwVulkanSupported())
		return static_cast<int>(ErrorType::GlfwVulkanNotFound);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(1024, 768, "Engine", nullptr, nullptr);

	std::vector<vk::ExtensionProperties> extensionProperties = vk::enumerateInstanceExtensionProperties();
	std::vector<const char*> availableExtensions;
	for (vk::ExtensionProperties property : extensionProperties)
		availableExtensions.push_back(property.extensionName);

	std::vector<vk::LayerProperties> layerProperties = vk::enumerateInstanceLayerProperties();
	std::vector<const char*> availableLayers;
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

	vk::UniqueInstance instance;
	try {
		instance = vk::createInstanceUnique(instanceInfo);
	}
	catch (std::runtime_error e) {
		return static_cast<int>(ErrorType::VulkanInstanceCreationFailed);
	}

	vk::UniqueSurfaceKHR surface;
#if defined(__linux__)
	// Determine which windowing API is being used via glfwRequiredExtensions.
#elif defined(_WIN32)
	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.setHinstance(GetModuleHandle(nullptr));
	surfaceCreateInfo.setHwnd(glfwGetWin32Window(window));
	surface = instance->createWin32SurfaceKHRUnique(surfaceCreateInfo);
#endif

	vk::PhysicalDevice physicalDevice;
	{
		std::vector<vk::PhysicalDevice> physicalDevices;
		std::vector<vk::PhysicalDeviceFeatures> features;
		std::vector<vk::PhysicalDeviceProperties> properties;
		std::vector<std::vector<vk::QueueFamilyProperties>> queueFamilies;
		std::vector<vk::SurfaceCapabilitiesKHR> capabilities;
		std::vector<std::vector<vk::SurfaceFormatKHR>> surfaceFormats;
		std::vector<std::vector<vk::PresentModeKHR>> surfacePresentModes;

		physicalDevices = instance->enumeratePhysicalDevices();
		uint32_t physicalDeviceIndex = 0;
		for (vk::PhysicalDevice physicalDevice : physicalDevices) {
			features.push_back(physicalDevice.getFeatures());
			queueFamilies.push_back(physicalDevice.getQueueFamilyProperties());
			properties.push_back(physicalDevice.getProperties());
		
			for (int i = 0; i < queueFamilies[physicalDeviceIndex].size(); i++) {
				// Only graphics queues should be checked.
				if (queueFamilies[physicalDeviceIndex][i].queueFlags & vk::QueueFlagBits::eGraphics) {
					// Check for surface support.
					VkBool32 supportsSurface = physicalDevice.getSurfaceSupportKHR(i, surface.get());
					if (supportsSurface = VK_TRUE) {
						// If we have surface support, we will query its features.
						capabilities.push_back(physicalDevice.getSurfaceCapabilitiesKHR(surface.get()));
						surfaceFormats.push_back(physicalDevice.getSurfaceFormatsKHR(surface.get()));
						surfacePresentModes.push_back(physicalDevice.getSurfacePresentModesKHR(surface.get()));
					}
					else {
						// Otherwise we will need to push an empty set of objects to keep the index correct.
						capabilities.push_back(vk::SurfaceCapabilitiesKHR());
						surfaceFormats.push_back(std::vector<vk::SurfaceFormatKHR>());
						surfacePresentModes.push_back(std::vector<vk::PresentModeKHR>());
					}
				}
			}
			physicalDeviceIndex++;
		}
	}

	//Renderer renderer(physicalDevice, surface);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}