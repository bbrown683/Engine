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

#include "renderable_vk.hpp"

#include "driver_vk.hpp"

RenderableVk::RenderableVk(DriverVk* pDriver) : m_pDriver(pDriver) {}

RenderableVk::~RenderableVk() {}

bool RenderableVk::build() {
	vk::CommandBufferAllocateInfo allocateInfo;
	allocateInfo.commandPool = m_pDriver->getCommandPool().get();
	allocateInfo.commandBufferCount = 1;
	allocateInfo.level = vk::CommandBufferLevel::eSecondary;
	m_pSecondaryBuffer = std::move(m_pDriver->getDevice()->allocateCommandBuffersUnique(allocateInfo).front());
	
	vk::CommandBufferInheritanceInfo inheritInfo;
	inheritInfo.framebuffer = m_pDriver->getCurrentFramebuffer().get();
	inheritInfo.renderPass = m_pDriver->getRenderPass().get();

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.pInheritanceInfo = &inheritInfo;
	m_pSecondaryBuffer->begin(beginInfo);

	m_pSecondaryBuffer->end();
	return false;
}

bool RenderableVk::attachShader(const char* pFilename, ShaderStage stage) {
    return true;
}

bool RenderableVk::setIndices(std::vector<uint16_t> indices) {
    return false;
}

bool RenderableVk::setVertices(std::vector<Vertex> vertices) {
	vk::BufferCreateInfo bufferInfo;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;
	bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
	bufferInfo.size = static_cast<vk::DeviceSize>(sizeof Vertex * vertices.size());
	m_pVertexBuffer = m_pDriver->getDevice()->createBufferUnique(bufferInfo);

	vk::MemoryRequirements memoryRequirements = m_pDriver->getDevice()->getBufferMemoryRequirements(m_pVertexBuffer.get());
	vk::MemoryAllocateInfo allocateInfo;
	allocateInfo.allocationSize = memoryRequirements.size;
	
	m_pDriver->getDevice()->allocateMemoryUnique(allocateInfo);
    return false;
}
