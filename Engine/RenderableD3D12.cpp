#include "RenderableD3D12.hpp"
#include "DriverD3D12.hpp"

RenderableD3D12::RenderableD3D12(DriverD3D12* pDriver) {}

bool RenderableD3D12::attachShader(const char * filename, ShaderStage stage) {
    return false;
}

bool RenderableD3D12::setIndexBuffer(std::vector<uint16_t> indices) {
    return false;
}

bool RenderableD3D12::setVertexBuffer(std::vector<uint32_t> vertices) {
    return false;
}
