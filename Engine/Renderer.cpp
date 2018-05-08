#include "Renderer.hpp"

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>

RenderingInstance::RenderingInstance(GLFWwindow* window) {
	vk::ApplicationInfo appInfo;
#ifdef VK_API_VERSION_1_1
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

	for (const char* extension : instanceExtensions)
		std::cout << extension << std::endl;
 
	vk::InstanceCreateInfo instanceInfo;
	instanceInfo.setPApplicationInfo(&appInfo);
	instanceInfo.setEnabledExtensionCount(static_cast<uint32_t>(instanceExtensions.size()));
	instanceInfo.setPpEnabledExtensionNames(instanceExtensions.data());
	instanceInfo.setEnabledLayerCount(static_cast<uint32_t>(instanceLayers.size()));
	instanceInfo.setPpEnabledLayerNames(instanceLayers.data());

	instance = vk::createInstanceUnique(instanceInfo);
	physicalDevices = instance->enumeratePhysicalDevices();

#if defined(__linux__)
	// Detect MIR, Xcb, Wayland.
#elif defined(_WIN32)
	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.setHinstance(GetModuleHandle(nullptr));
	surfaceCreateInfo.setHwnd(glfwGetWin32Window(window));
	surface = instance->createWin32SurfaceKHRUnique(surfaceCreateInfo);
#endif
}

vk::UniqueInstance& RenderingInstance::getInstance() {
	return instance;
}

std::vector<vk::PhysicalDevice>& RenderingInstance::getAllPhysicalDevices() {
	return physicalDevices;
}

vk::UniqueSurfaceKHR& RenderingInstance::getSurface() {
	return surface;
}

RenderingDevice::RenderingDevice(vk::PhysicalDevice& physicalDevice, vk::UniqueSurfaceKHR& surface)
	: physicalDevice(physicalDevice), surface(surface) {
	vk::DeviceCreateInfo deviceInfo;
	vk::SwapchainCreateInfoKHR swapchainInfo;
	threadCount = std::thread::hardware_concurrency();
}

bool RenderingDevice::present() {
	return false;
}

bool RenderingDevice::updateSwapchain() {
	return false;
}

vk::UniqueDevice& RenderingDevice::getDevice() {
	return device;
}

vk::UniqueSwapchainKHR& RenderingDevice::getSwapchain() {
	return swapchain;
}

ThreadedCommandPool::ThreadedCommandPool(vk::UniqueDevice& device) {
	vk::CommandPoolCreateInfo commandPoolInfo;
}
