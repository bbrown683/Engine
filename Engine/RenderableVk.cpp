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

#include "RenderableVk.hpp"

#include "DriverVk.hpp"
#include "System.hpp"

RenderableVk::RenderableVk(DriverVk* pDriver) : m_pDriver(pDriver) {}

bool RenderableVk::build() {
	return false;
}

bool RenderableVk::attachShader(const char* pFilename, ShaderStage stage) {
	vk::UniqueShaderModule module = m_pDriver->getShaderModuleFromFile(pFilename);

    vk::PipelineShaderStageCreateInfo shaderStageInfo;
    shaderStageInfo.module = module.get();
    shaderStageInfo.pName = "main";
    switch (stage) {
    case ShaderStage::Fragment: shaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment; break;
    case ShaderStage::Vertex: shaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    }
    m_ShaderStages.push_back(std::move(shaderStageInfo));
    return true;
}

bool RenderableVk::setIndices(std::vector<uint16_t> indices) {
    return false;
}

bool RenderableVk::setVertices(std::vector<Vertex> vertices) {
	vk::DeviceSize bufferSize = sizeof(uint32_t) * vertices.size();
	m_pDriver->createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    return false;
}
