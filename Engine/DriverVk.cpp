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
#include <SDL2/SDL_syswm.h>

#include "thirdparty/loguru.hpp"
#include "RenderableVk.hpp"
#include "System.hpp"

DriverVk::DriverVk(const SDL_Window* pWindow) : Driver(pWindow) {
	m_ImageFormat = vk::Format::eUndefined;
	m_ImageCount = 2;
	m_CurrentImage = 0;
	m_pImageViews = std::vector<vk::UniqueImageView>(m_ImageCount);
	m_pFramebuffers = std::vector<vk::UniqueFramebuffer>(m_ImageCount);
	m_ClearColor = { 0.1f, 0.3f, 0.5f, 1.0f };
}

bool DriverVk::initialize() {
	try {
		std::vector<vk::ExtensionProperties> extensionProperties = vk::enumerateInstanceExtensionProperties();

		bool surfaceKHRSupport = false, wmKHRSupport = false;
		for (vk::ExtensionProperties extension : extensionProperties) {
			if (strcmp(extension.extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0)
				surfaceKHRSupport = true;
			if (strcmp(extension.extensionName,
#ifdef VK_USE_PLATFORM_MIR_KHR
				VK_KHR_MIR_SURFACE_EXTENSION_NAME) == 0)
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
				VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME) == 0)
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
				VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
				VK_KHR_XLIB_SURFACE_EXTENSION_NAME) == 0)
#endif
				wmKHRSupport = true;
		}

		if (!surfaceKHRSupport || !wmKHRSupport) {
			LOG_F(FATAL, "Vulkan driver does not support rendering to a surface!");
			return false;
		}

		std::vector<vk::LayerProperties> layerProperties = vk::enumerateInstanceLayerProperties();

		vk::ApplicationInfo appInfo;
		appInfo.apiVersion = VK_API_VERSION_1_0;

		std::vector<const char*> instanceExtensions;
		std::vector<const char*> instanceLayers;

#ifdef _DEBUG
		instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

		instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		if (!SDL_GetWindowWMInfo(const_cast<SDL_Window*>(getWindow()), &wmInfo))
			return false;

		switch (wmInfo.subsystem) {
#ifdef VK_USE_PLATFORM_MIR_KHR
		case SDL_SYSWM_MIR: instanceExtensions.push_back(VK_KHR_MIR_SURFACE_EXTENSION_NAME); break;
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
		case SDL_SYSWM_WAYLAND: instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME); break;
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
		case SDL_SYSWM_WINDOWS: instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME); break;
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
		case SDL_SYSWM_X11: instanceExtensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME); break;
#endif
		default: LOG_F(FATAL, "Could not detect a supported Window Manager!"); return false;
		}

		vk::InstanceCreateInfo instanceInfo;
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
		instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();
		instanceInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
		instanceInfo.ppEnabledLayerNames = instanceLayers.data();
		m_pInstance = vk::createInstanceUnique(instanceInfo);

#ifdef VK_USE_PLATFORM_MIR_KHR
		vk::MIRSurfaceCreateInfoKHR surfaceCreateInfo;
		surfaceCreateInfo.connection = wmInfo.info.mir.connection;
		surfaceCreateInfo.surface = wmInfo.info.mir.surface;
		m_pSurface = m_pInstance->createMirSurfaceKHRUnique(surfaceCreateInfo);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
		vk::WaylandSurfaceCreateInfoKHR surfaceCreateInfo;
		surfaceCreateInfo.display = wmInfo.info.wl.display;
		surfaceCreateInfo.surface = wmInfo.info.wl.surface;
		m_pSurface = m_pInstance->createWaylandSurfaceKHRUnique(surfaceCreateInfo);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
		vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo;
		surfaceCreateInfo.hinstance = wmInfo.info.win.hinstance;
		surfaceCreateInfo.hwnd = wmInfo.info.win.window;
		m_pSurface = m_pInstance->createWin32SurfaceKHRUnique(surfaceCreateInfo);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
		vk::XCBSurfaceCreateInfoKHR surfaceCreateInfo;
		surfaceCreateInfo.dpy = wmInfo.info.x11.display;
		surfaceCreateInfo.window = wmInfo.info.x11.window;
		m_pSurface = m_pInstance->createXlibSurfaceKHRUnique(surfaceCreateInfo);
#else
		logger.logMessage("Could not detect a supported Window Manager!");
		return false;
#endif

		m_PhysicalDevices = m_pInstance->enumeratePhysicalDevices();

		LOG_F(INFO, "Enumerating Physical Devices:");
		uint32_t counter = 0;
		for (vk::PhysicalDevice physicalDevice : m_PhysicalDevices) {
			vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();

			Gpu gpu;
			gpu.id = counter++;
			std::strcpy(gpu.name, properties.deviceName);
			gpu.vendorId = properties.vendorID;
			gpu.deviceId = properties.deviceID;
			gpu.memory = properties.limits.maxMemoryAllocationCount;
			gpu.software = properties.deviceType == vk::PhysicalDeviceType::eVirtualGpu ? true : false;

			LOG_F(INFO, "\t[%u]: %s", gpu.id, gpu.name);
			LOG_F(INFO, "\t\tVideoMemory: %u", gpu.memory);
			LOG_F(INFO, "\t\tVendorId: %u", gpu.vendorId);
			LOG_F(INFO, "\t\tDeviceId: %u", gpu.deviceId);
			addGpu(gpu);
		}
	} catch (vk::SystemError e) {
		LOG_F(FATAL, "Vulkan driver failure: %i - %s", e.code(), e.what());
		return false;
	}
	return !m_PhysicalDevices.empty();
}

bool DriverVk::selectGpu(uint32_t id) {
	try {
		// id Does not correlate to a proper GPU.
		if (id >= m_PhysicalDevices.size())
			return false;

		std::vector<vk::ExtensionProperties> deviceExtensions = m_PhysicalDevices[id].enumerateDeviceExtensionProperties();

		// Check for VK_KHR_swapchain extension.
		bool swapchainKHRSupport = false;
		for (vk::ExtensionProperties deviceExtension : deviceExtensions) {
			if (std::strcmp(deviceExtension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
				swapchainKHRSupport = true;
		}

		if (!swapchainKHRSupport) {
			LOG_F(FATAL, "Hardware device does not support presenting to a surface!");
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

		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); i++) {
			// Only graphics queues should be checked.
			if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
				graphicsSupport.push_back(i);

			// Check for surface support.
			VkBool32 supportsSurface = m_PhysicalDevices[id].getSurfaceSupportKHR(i, m_pSurface.get());
			if (supportsSurface) {
				surfaceCapabilities = m_PhysicalDevices[id].getSurfaceCapabilitiesKHR(m_pSurface.get());
				surfaceFormats = m_PhysicalDevices[id].getSurfaceFormatsKHR(m_pSurface.get());
				surfacePresentModes = m_PhysicalDevices[id].getSurfacePresentModesKHR(m_pSurface.get());

				// Found a queue family which supports presentation.
				surfaceSupport.push_back(i);
			}
		}

		// Graphics or Surface are not supported.
		if (graphicsSupport.empty() || surfaceSupport.empty()) {
			LOG_F(FATAL, "Hardware device does not support drawing or surface operations!");
			return false;
		} else {
			// Pick first queue family which supports both.
			bool foundQueueIndex = false;
			for (size_t i = 0; i < graphicsSupport.size() || foundQueueIndex != true; i++) {
				for (size_t j = 0; j < surfaceSupport.size(); j++) {
					if (graphicsSupport[i] == surfaceSupport[j]) {
						m_QueueFamilyIndex = graphicsSupport[i];
						foundQueueIndex = true;
						break;
					}
				}
			}
		}

		float priority = 1.0f;
		vk::DeviceQueueCreateInfo deviceQueueInfo;
		deviceQueueInfo.queueCount = 1;
		deviceQueueInfo.queueFamilyIndex = m_QueueFamilyIndex;
		deviceQueueInfo.pQueuePriorities = &priority;

		std::vector<const char*> enabledDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		vk::PhysicalDeviceFeatures enabledFeatures = { enabledFeatures.samplerAnisotropy = VK_TRUE };

		// Create device.
		vk::DeviceCreateInfo deviceInfo;
		deviceInfo.pEnabledFeatures = &enabledFeatures;
		deviceInfo.enabledLayerCount = 0;
		deviceInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();
		deviceInfo.enabledExtensionCount = static_cast<uint32_t>(enabledDeviceExtensions.size());
		deviceInfo.pQueueCreateInfos = &deviceQueueInfo;
		deviceInfo.queueCreateInfoCount = 1;
		m_pDevice = m_PhysicalDevices[id].createDeviceUnique(deviceInfo);

		// Select present mode. Vsync is Fifo, Triple buffering is Mailbox. Immediate is uncapped.
		vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
		//if (surfaceCapabilities.minImageCount > 2)
		//	for (vk::PresentModeKHR tempPresentMode : surfacePresentModes)
		//		if (tempPresentMode == vk::PresentModeKHR::eMailbox)
		//			presentMode = tempPresentMode;

		// Select default swapchain format.
		m_ImageFormat = vk::Format::eUndefined;

		// Check to see if the driver lets us select which one we want.
		if (surfaceFormats.size() == 1 && surfaceFormats[0].format == vk::Format::eUndefined)
			m_ImageFormat = vk::Format::eR8G8B8A8Unorm;
		else {
			// Iterate through each format and check to see if it has the format we want.
			for (vk::SurfaceFormatKHR surfaceFormat : surfaceFormats)
				if (surfaceFormat.format == vk::Format::eR8G8B8A8Unorm)
					m_ImageFormat = surfaceFormat.format;
			// If we still didnt find a format just pick the first one we get.
			if (m_ImageFormat == vk::Format::eUndefined)
				m_ImageFormat = surfaceFormats.front().format;
		}

		// Create the swapchain for the surface.
		vk::SwapchainCreateInfoKHR swapchainInfo;
		swapchainInfo.clipped = VK_TRUE;
		swapchainInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapchainInfo.minImageCount = m_ImageCount;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageExtent = surfaceCapabilities.currentExtent;
		swapchainInfo.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
		swapchainInfo.imageFormat = m_ImageFormat;
		swapchainInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
		swapchainInfo.presentMode = presentMode;
		swapchainInfo.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
		swapchainInfo.surface = m_pSurface.get();
		m_pSwapchain = m_pDevice->createSwapchainKHRUnique(swapchainInfo);

		std::vector<vk::Image> swapchainImages = m_pDevice->getSwapchainImagesKHR(m_pSwapchain.get());
		
		vk::ImageViewCreateInfo viewInfo;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = m_ImageFormat;
		viewInfo.components.r = vk::ComponentSwizzle::eR;
		viewInfo.components.g = vk::ComponentSwizzle::eG;
		viewInfo.components.b = vk::ComponentSwizzle::eB;
		viewInfo.components.a = vk::ComponentSwizzle::eA;
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		for (uint32_t i = 0; i < m_ImageCount; i++) {
			viewInfo.image = swapchainImages[i];
			m_pImageViews[i] = std::move(m_pDevice->createImageViewUnique(viewInfo));
		}

		// Grab a queue related to our device.
		m_Queue = m_pDevice->getQueue(m_QueueFamilyIndex, 0);

		// Create the command pool to store our command buffers: both primary and secondary.
		vk::CommandPoolCreateInfo poolInfo;
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		poolInfo.queueFamilyIndex = m_QueueFamilyIndex;
		m_pCommandPool = m_pDevice->createCommandPoolUnique(poolInfo);

		// Allocate primary command buffer.
		vk::CommandBufferAllocateInfo allocateInfo;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.commandPool = m_pCommandPool.get();
		allocateInfo.level = vk::CommandBufferLevel::ePrimary;
		m_pCommandBuffer = std::move(m_pDevice->allocateCommandBuffersUnique(allocateInfo).front());

		// Create the color attachment which will be used for the render pass.
		vk::AttachmentDescription colorAttachment;
		colorAttachment.format = m_ImageFormat;
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
		
		vk::AttachmentReference colorAttachmentRef;
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		// Create the subpass descriptor.
		vk::SubpassDescription subpassDesc;
		subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpassDesc.colorAttachmentCount = 1;
		subpassDesc.pColorAttachments = &colorAttachmentRef;
		
		// Create the render pass.
		vk::RenderPassCreateInfo passInfo;
		passInfo.attachmentCount = 1;
		passInfo.pAttachments = &colorAttachment;
		passInfo.subpassCount = 1;
		passInfo.pSubpasses = &subpassDesc;
		m_pRenderPass = m_pDevice->createRenderPassUnique(passInfo);

		vk::FramebufferCreateInfo framebufferInfo;
		framebufferInfo.renderPass = m_pRenderPass.get();
		framebufferInfo.width = surfaceCapabilities.currentExtent.width;
		framebufferInfo.height = surfaceCapabilities.currentExtent.height;
		framebufferInfo.layers = 1;
		for (uint32_t i = 0; i < m_ImageCount; i++) {
			vk::ImageView attachmentView = m_pImageViews[i].get();
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = &attachmentView;
			m_pFramebuffers[i] = std::move(m_pDevice->createFramebufferUnique(framebufferInfo));
		}

		vk::FenceCreateInfo fenceInfo;
		m_pFence = m_pDevice->createFenceUnique(fenceInfo);

		vk::SemaphoreCreateInfo semaphoreInfo;
		m_pSemaphore = m_pDevice->createSemaphoreUnique(semaphoreInfo);
	} catch (vk::SystemError e) {
		LOG_F(FATAL, "Vulkan driver failure: %i - %s", e.code(), e.what());
		return false;
	}
	LOG_F(INFO, "Vulkan driver was successfully initialized.");
	return true;
}

bool DriverVk::prepareFrame() {
	try {
		auto acquireResult = m_pDevice->acquireNextImageKHR(m_pSwapchain.get(), UINT64_MAX, m_pSemaphore.get(), nullptr);
		if (acquireResult.result != vk::Result::eSuccess) {
			LOG_F(WARNING, "Failed to grab available swapchain image.");
			return false;
		}
		else
			m_CurrentImage = acquireResult.value;

		// Begin recording.
		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		m_pCommandBuffer->begin(beginInfo);

		std::array<vk::ClearValue, 1> clearValue;
		clearValue[0].color.setFloat32(m_ClearColor);

		vk::RenderPassBeginInfo passBeginInfo;
		passBeginInfo.clearValueCount = clearValue.size();
		passBeginInfo.pClearValues = clearValue.data();
		passBeginInfo.framebuffer = m_pFramebuffers[m_CurrentImage].get();
		passBeginInfo.renderPass = m_pRenderPass.get();
		// TODO: update render area based on swapchain dimensions when resized.
		passBeginInfo.renderArea = vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(1024, 768));

		m_pCommandBuffer->beginRenderPass(passBeginInfo, vk::SubpassContents::eInline);
		m_pCommandBuffer->endRenderPass();

		// Stop recording.
		m_pCommandBuffer->end();
	} catch (vk::SystemError e) {
		LOG_F(FATAL, "Vulkan driver failure: %i - %s", e.code(), e.what());
		return false;
	}
	return true;
}

bool DriverVk::presentFrame() {
	try {
		const std::vector<vk::PipelineStageFlags> waitStages = {
			vk::PipelineStageFlagBits::eAllGraphics
		};

		// We are only submitting the primary command list.
		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_pCommandBuffer.get();
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &m_pSemaphore.get();
		submitInfo.pWaitDstStageMask = waitStages.data();
		m_Queue.submit(submitInfo, m_pFence.get());

		// Preprare to present to the queue.
		vk::PresentInfoKHR presentInfo;
		presentInfo.pImageIndices = &m_CurrentImage;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_pSwapchain.get();
		m_Queue.presentKHR(presentInfo);

		m_pDevice->waitForFences(m_pFence.get(), true, UINT64_MAX);
		m_pDevice->resetFences(m_pFence.get());
	} catch (vk::SystemError e) {
		LOG_F(FATAL, "Vulkan driver failure: %i - %s", e.code(), e.what());
		return false;
	}
	return true;
}

std::unique_ptr<Renderable> DriverVk::createRenderable() {
    return std::make_unique<RenderableVk>(this);
}

const vk::UniqueDevice& DriverVk::getDevice() const {
    return m_pDevice;
}

const vk::UniqueCommandPool & DriverVk::getCommandPool() const {
	return m_pCommandPool;
}

const vk::UniqueCommandBuffer& DriverVk::getCommandBuffer() const {
    return m_pCommandBuffer;
}

const vk::UniqueSwapchainKHR& DriverVk::getSwapchain() const {
    return m_pSwapchain;
}

vk::UniqueShaderModule DriverVk::getShaderModuleFromFile(const char* pFilename) {
	const char* pModuleName = std::strcat(const_cast<char*>(pFilename), ".spv");
	auto file = System::readFile(pModuleName);
	vk::ShaderModuleCreateInfo moduleInfo;
	moduleInfo.pCode = reinterpret_cast<const uint32_t*>(file.first);
	moduleInfo.codeSize = file.second;

	return m_pDevice->createShaderModuleUnique(moduleInfo);
}

void DriverVk::createBuffer(vk::DeviceSize size, vk::BufferUsageFlagBits usage, vk::MemoryPropertyFlags properties) {
	vk::BufferCreateInfo bufferInfo;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;
}
