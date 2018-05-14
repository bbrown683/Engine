#include "RenderableVk.hpp"
#include "DriverVk.hpp"

RenderableVk::RenderableVk(DriverVk* pDriver) : m_pDriver(pDriver) {}

bool RenderableVk::attachShader(const char* filename, ShaderStage stage) {
    return false;
}

bool RenderableVk::setIndexBuffer(std::vector<uint16_t> indices) {
    return false;
}

bool RenderableVk::setVertexBuffer(std::vector<uint32_t> vertices) {
    return false;
}
