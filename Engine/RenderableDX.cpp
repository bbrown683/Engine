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

#include "RenderableDX.hpp"

#include "DriverDX.hpp"
#include "thirdparty/d3dx12.h"

RenderableDX::RenderableDX(DriverDX* pDriver) : m_pDriver(pDriver) {
	//if(FAILED(m_pDriver->getDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE,
	//	m_pDriver->getCommandAllocator().Get(), nullptr, IID_PPV_ARGS(&m_pBundle))));
}

bool RenderableDX::execute() {
	// Define the vertex input layout.
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputElementDescs.data(), inputElementDescs.size() };
	psoDesc.pRootSignature = m_pDriver->getRootSignature().Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_pVertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_pPixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	return false;
}

bool RenderableDX::attachShader(const char* pFilename, ShaderStage stage) {
    const char* pBlobName = std::strcat(const_cast<char*>(pFilename), ".cso");
    const char* target;
    switch (stage) {
    case ShaderStage::Fragment: target = "ps_5_0"; break;
    case ShaderStage::Vertex: target = "vs_5_0"; break;
    }
    wchar_t pBlobNameWide[64];
    size_t length = std::mbstowcs(pBlobNameWide, pBlobName, 64);
    if (length != -1)
        pBlobNameWide[length] = '\0';
    
    ID3DBlob* pBlob;
    if (FAILED(D3DReadFileToBlob(pBlobNameWide, &pBlob)))
        return false;

    return true;
}

bool RenderableDX::setIndexBuffer(std::vector<uint16_t> indices) {
    return false;
}

bool RenderableDX::setVertexBuffer(std::vector<uint32_t> vertices) {
    return false;
}

const ComPtr<ID3D12GraphicsCommandList>& RenderableDX::getBundle() const {
	return m_pBundle;
}
