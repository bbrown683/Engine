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

#include <memory>
#include <unordered_map>
#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include "renderer/renderable.hpp"

class DriverVk;
class RenderableVk : public Renderable {
public:
    RenderableVk(DriverVk* pDriver);
	~RenderableVk();
    bool build() override;
    bool attachShader(const char* pFilename, ShaderStage stage) override;
    bool setIndices(std::vector<uint16_t> indices) override;
    bool setVertices(std::vector<Vertex> vertices) override;
private:
    DriverVk* m_pDriver;
	vk::UniqueCommandBuffer m_pSecondaryBuffer;
	vk::UniqueBuffer m_pIndexBuffer;
	vk::UniqueBuffer m_pVertexBuffer;
	vk::UniqueDeviceMemory m_pIndexBufferMemory;
	vk::UniqueDeviceMemory m_pVertexBufferMemory;
    vk::UniquePipeline m_pPipeline;
    vk::UniqueRenderPass m_pRenderPass;
    std::vector<vk::PipelineShaderStageCreateInfo> m_ShaderStages;
};