#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <vector>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <wrl/client.h>
using namespace Microsoft::WRL;

#include "Renderable.hpp"

class DriverD3D12;
class RenderableD3D12 : public Renderable {
public:
    RenderableD3D12(DriverD3D12* pDriver);
    bool attachShader(const char* filename, ShaderStage stage) override;
    bool setIndexBuffer(std::vector<uint16_t> indices) override;
    bool setVertexBuffer(std::vector<uint32_t> vertices) override;
private:
    ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
};