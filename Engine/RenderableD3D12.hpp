#pragma once

#include <vector>

#include "Renderable.hpp"

class DriverD3D12;
class RenderableD3D12 : public Renderable {
public:
    RenderableD3D12(DriverD3D12* pDriver);
    bool attachShader(const char* filename, ShaderStage stage) override;
    bool setIndexBuffer(std::vector<uint16_t> indices) override;
    bool setVertexBuffer(std::vector<uint32_t> vertices) override;
private:

};