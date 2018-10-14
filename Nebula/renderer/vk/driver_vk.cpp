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

#include "driver_vk.hpp"

#include <iostream>

#include <SDL2/SDL_syswm.h>
#include "thirdparty/loguru/loguru.hpp"

#include "renderable_vk.hpp"
#include "helper_vk.hpp"
#include "System.hpp"

DriverVk::DriverVk(const SDL_Window* pWindow) : Driver(pWindow) {
	m_ColorFormat = vk::Format::eUndefined;
	m_DepthStencilFormat = vk::Format::eUndefined;
	m_ImageCount = 2;
	m_CurrentImage = 0;
	m_pColorImageViews = std::vector<vk::UniqueImageView>(m_ImageCount);
	m_pFramebuffers = std::vector<vk::UniqueFramebuffer>(m_ImageCount);
	m_ClearColor = { 0.1f, 0.3f, 0.5f, 1.0f };
}

bool DriverVk::initialize() {
	try {
		if (!HelperVk::hasRequiredInstanceExtensions())
			return false;

		std::vector<vk::LayerProperties> layerProperties = vk::enumerateInstanceLayerProperties();

		vk::ApplicationInfo appInfo;
		appInfo.apiVersion = VK_API_VERSION_1_0;

		std::vector<const char*> instanceExtensions;
		std::vector<const char*> instanceLayers;

		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		if (!SDL_GetWindowWMInfo(const_cast<SDL_Window*>(getWindow()), &wmInfo))
			return false;

		m_pInstance = HelperVk::createInstance(wmInfo);
		m_pSurface = HelperVk::createSurface(m_pInstance.get(), wmInfo);

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
	
		// Grab physical device and verify support.
		vk::PhysicalDevice physicalDevice = m_PhysicalDevices[id];
		if (!HelperVk::hasRequiredDeviceExtensionsAndFeatures(physicalDevice))
			return false;

		// Grab queue family index which supports graphics and surface operations.
		if ((m_QueueFamilyIndex = HelperVk::selectQueueFamilyIndex(physicalDevice, m_pSurface.get())) == std::numeric_limits<uint32_t>::max())
			return false;

		m_pDevice = HelperVk::createDevice(physicalDevice, m_QueueFamilyIndex);

		m_ColorFormat = HelperVk::selectColorFormat(physicalDevice, m_pSurface.get());
		vk::PresentModeKHR presentMode = HelperVk::selectPresentMode(physicalDevice, m_pSurface.get());
		vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(m_pSurface.get());
		m_SurfaceDimensions = surfaceCapabilities.currentExtent;
		m_pSwapchain = HelperVk::createSwapchain(m_pDevice.get(), m_pSurface.get(), m_SurfaceDimensions, m_ImageCount, m_ColorFormat, presentMode);

		// Grab image views for color buffer.
		m_pColorImageViews = std::move(HelperVk::createImageViews(m_pDevice.get(), m_pSwapchain.get(), nullptr, m_ColorFormat));
		if (m_pColorImageViews.empty())
			return false;

		// Grab depth stencil format.
		if ((m_DepthStencilFormat = HelperVk::selectDepthStencilFormat(physicalDevice)) == vk::Format::eUndefined) {
			LOG_F(FATAL, "Failed to find a suitable depth-stencil format.");
			return false;
		}

		// Create image for our depth stencil view.
		vk::ImageCreateInfo imageInfo;
		imageInfo.format = m_DepthStencilFormat;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent = vk::Extent3D(m_SurfaceDimensions, 1);
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc;
		vk::Image depthStencilImage = m_pDevice->createImage(imageInfo);

		vk::MemoryRequirements memoryRequirements = m_pDevice->getImageMemoryRequirements(depthStencilImage);
		uint32_t memoryTypeIndex = std::numeric_limits<uint32_t>::max();
		if ((memoryTypeIndex = HelperVk::getMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits,
			vk::MemoryPropertyFlagBits::eDeviceLocal)) == std::numeric_limits<uint32_t>::max())
			return false;

		// Parameters for allocating image to memory.
		vk::MemoryAllocateInfo memoryAllocateInfo;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

		// Bind image to memory and create a view for it.
		m_pDepthStencilImage = m_pDevice->allocateMemoryUnique(memoryAllocateInfo);
		m_pDevice->bindImageMemory(depthStencilImage, m_pDepthStencilImage.get(), 0);
		m_pDepthStencilView = std::move(HelperVk::createImageViews(m_pDevice.get(), m_pSwapchain.get(), depthStencilImage, m_DepthStencilFormat,
			vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil).front());
		
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

		// 0: color, 1: depth-stencil
		std::array<vk::AttachmentDescription, 2> attachmentDescs;
		attachmentDescs[0].format = m_ColorFormat;
		attachmentDescs[0].samples = vk::SampleCountFlagBits::e1;
		attachmentDescs[0].loadOp = vk::AttachmentLoadOp::eClear;
		attachmentDescs[0].storeOp = vk::AttachmentStoreOp::eStore;
		attachmentDescs[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachmentDescs[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachmentDescs[0].initialLayout = vk::ImageLayout::eUndefined;
		attachmentDescs[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

		attachmentDescs[1].format = m_DepthStencilFormat;
		attachmentDescs[1].samples = vk::SampleCountFlagBits::e1;
		attachmentDescs[1].loadOp = vk::AttachmentLoadOp::eClear;
		attachmentDescs[1].initialLayout = vk::ImageLayout::eUndefined;
		attachmentDescs[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachmentDescs[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachmentDescs[1].initialLayout = vk::ImageLayout::eUndefined;
		attachmentDescs[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		
		// Reference for color attachment.
		vk::AttachmentReference colorRef;
		colorRef.attachment = 0;
		colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		// Reference for depth attachment.
		vk::AttachmentReference depthStencilRef;
		depthStencilRef.attachment = 1;
		depthStencilRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		// Create the subpass descriptor.
		vk::SubpassDescription subpassDesc;
		subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpassDesc.colorAttachmentCount = 1;
		subpassDesc.pColorAttachments = &colorRef;
		subpassDesc.pDepthStencilAttachment = &depthStencilRef;
		
		// Create the render pass.
		vk::RenderPassCreateInfo passInfo;
		passInfo.attachmentCount = attachmentDescs.size();
		passInfo.pAttachments = attachmentDescs.data();
		passInfo.subpassCount = 1;
		passInfo.pSubpasses = &subpassDesc;
		m_pRenderPass = m_pDevice->createRenderPassUnique(passInfo);

		// Create a framebuffer for each image image in the swapchain.
		vk::FramebufferCreateInfo framebufferInfo;
		framebufferInfo.renderPass = m_pRenderPass.get();
		framebufferInfo.width = m_SurfaceDimensions.width;
		framebufferInfo.height = m_SurfaceDimensions.height;
		framebufferInfo.layers = 1;
		for (uint32_t i = 0; i < m_ImageCount; i++) {
			std::array<vk::ImageView, 2> attachmentViews;
			attachmentViews[0] = m_pColorImageViews[i].get();
			attachmentViews[1] = m_pDepthStencilView.get();

			framebufferInfo.attachmentCount = attachmentViews.size();
			framebufferInfo.pAttachments = attachmentViews.data();
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

		std::array<vk::ClearValue, 2> clearValue;
		clearValue[0].color.setFloat32(m_ClearColor);
		clearValue[1].depthStencil = { 1.0f, 0 };

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

void DriverVk::addRenderable(Renderable* renderable) {
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

const vk::UniqueFramebuffer & DriverVk::getCurrentFramebuffer() const {
	return m_pFramebuffers[m_CurrentImage];
}

const vk::UniqueRenderPass & DriverVk::getRenderPass() const {
	return m_pRenderPass;
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
