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

#pragma once

#include <array>
#include <memory>
#include <unordered_map>
#include <vector>


#include <SDL2/SDL_config.h>
#ifdef SDL_VIDEO_DRIVER_MIR
#define VK_USE_PLATFORM_MIR_KHR
#elif defined(SDL_VIDEO_DRIVER_WAYLAND)
#define VK_USE_PLATFORM_WAYLAND_KHR
#elif defined(SDL_VIDEO_DRIVER_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(SDL_VIDEO_DRIVER_X11)
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <vulkan/vulkan.hpp>

#include "Driver.hpp"

class DriverVk : public Driver {
public:
	DriverVk() = default;
    DriverVk(const SDL_Window* pWindow);

    // Inherited via IDriver
    bool initialize() override;
    bool selectGpu(uint32_t id) override;
	bool prepareFrame() override;
    bool presentFrame() override;
    std::unique_ptr<Renderable> createRenderable() override;

    const vk::UniqueDevice& getDevice() const;
	const vk::UniqueCommandPool& getCommandPool() const;
    const vk::UniqueCommandBuffer& getCommandBuffer() const;
    const vk::UniqueSwapchainKHR& getSwapchain() const;
	vk::UniqueShaderModule getShaderModuleFromFile(const char* pFilename);
	void createBuffer(vk::DeviceSize size, vk::BufferUsageFlagBits usage, vk::MemoryPropertyFlags properties);
private:
    vk::UniqueInstance m_pInstance;
    std::vector<vk::PhysicalDevice> m_PhysicalDevices;
    vk::UniqueSurfaceKHR m_pSurface;
    vk::UniqueDevice m_pDevice;
    vk::UniqueFence m_pFence;
    vk::UniqueSwapchainKHR m_pSwapchain;
	std::vector<vk::UniqueImageView> m_pImageViews;
	vk::Queue m_Queue;
	vk::UniqueCommandPool m_pCommandPool;
	vk::UniqueCommandBuffer m_pCommandBuffer;
	std::vector<vk::UniqueFramebuffer> m_pFramebuffers;
	vk::UniqueRenderPass m_pRenderPass;
    uint32_t m_QueueFamilyIndex;
	vk::Format m_ImageFormat;
    bool anisotropy;
    float maxAnisotropy;
	uint32_t m_ImageCount;
	uint32_t m_CurrentImage;
};