#pragma once

#define VULKAN_HPP_TYPESAFE_CONVERSION
#include <vulkan/vulkan.hpp>

class Renderer {
public:
	Renderer(vk::PhysicalDevice& physicalDevice, vk::UniqueSurfaceKHR& surface);
	void updateSwapchain();
	void present();
private:
	vk::UniqueDevice device;
	vk::UniqueCommandPool commandPool;
	vk::UniqueSwapchainKHR swapchain;
};