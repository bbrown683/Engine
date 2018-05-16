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

#include "RenderableD3D12.hpp"
#include "DriverD3D12.hpp"

RenderableD3D12::RenderableD3D12(DriverD3D12* pDriver) : m_pDriver(pDriver) {}

bool RenderableD3D12::attachShader(const char* filename, ShaderStage stage) {
    const char* target;
    switch (stage) {
    case ShaderStage::Fragment: target = "ps_5_0"; break;
    case ShaderStage::Geometry: target = "gs_5_0"; break;
    case ShaderStage::TesselationControl: target = "hs_5_0"; break;
    case ShaderStage::TesselationEvaluation: target = "ds_5_0"; break;
    case ShaderStage::Vertex: target = "vs_5_0"; break;
    }

    unsigned int flags;


#ifdef _DEBUG
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

    //D3DCompileFromFile(filename, nullptr, nullptr, "main", target, compileFlags, 0, , nullptr);
    return false;
}

bool RenderableD3D12::execute() {
    return false;
}

bool RenderableD3D12::setIndexBuffer(std::vector<uint16_t> indices) {
    return false;
}

bool RenderableD3D12::setVertexBuffer(std::vector<uint32_t> vertices) {
    return false;
}
