#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include "Renderable.hpp"

class DriverVk;
class RenderableVk : public Renderable {
public:
    RenderableVk(DriverVk* pDriver);
    bool attachShader(const char* filename, ShaderStage stage) override;
    bool setIndexBuffer(std::vector<uint16_t> indices) override;
    bool setVertexBuffer(std::vector<uint32_t> vertices) override;
private:
    DriverVk* m_pDriver;
    vk::UniqueCommandBuffer m_pCommandBuffer;
    vk::UniquePipeline m_Pipeline;
    vk::UniqueRenderPass m_RenderPass;
    std::vector<vk::PipelineShaderStageCreateInfo> m_ShaderStages;
};