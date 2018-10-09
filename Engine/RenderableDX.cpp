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
#include "thirdparty/d3dx12/d3dx12.h"
#include "thirdparty/loguru/loguru.hpp"

RenderableDX::RenderableDX(DriverDX* pDriver) : m_pDriver(pDriver) {}

bool RenderableDX::build() {
	if (m_pBundle) {
		if (FAILED(m_pBundle->Reset(m_pDriver->getBundledAllocator().Get(), nullptr)))
			return false;
	}

	// Define the vertex input layout.
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
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
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	// Create pipeline state.
	if (FAILED(m_pDriver->getDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState))))
		return false;
	
	// Create bundle with pipeline state.
	if (FAILED(m_pDriver->getDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE,
		m_pDriver->getBundledAllocator().Get(), m_pPipelineState.Get(), IID_PPV_ARGS(&m_pBundle))))
		return false;

	// Set bundle state and draw.
	//m_pBundle->SetGraphicsRootSignature(m_pDriver->getRootSignature().Get());
	m_pBundle->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pBundle->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	m_pBundle->DrawInstanced(3, 1, 0, 0);
	
	if (FAILED(m_pBundle->Close()))
		return false;
	return true;
}

bool RenderableDX::attachShader(const char* pFilename, ShaderStage stage) {
	ComPtr<ID3DBlob> pError;
	auto path = L"C:\\Users\\Ben\\Ivy3\\Engine\\shaders\\shaders.hlsl";
	if (FAILED(D3DCompileFromFile(path, nullptr, nullptr, "VSMain", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_pVertexShader, &pError)))
		return false;
	if (FAILED(D3DCompileFromFile(path, nullptr, nullptr, "PSMain", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_pPixelShader, &pError)))
		return false;
	return true;
}

bool RenderableDX::setIndices(std::vector<uint16_t> indices) {
    return false;
}

bool RenderableDX::setVertices(std::vector<Vertex> vertices) {
	UINT vertexBufferSize = sizeof(Vertex) * vertices.size();
	
	if(FAILED(m_pDriver->getDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, 
		nullptr, IID_PPV_ARGS(&m_pVertexBuffer))))
		return false;

	// Copy the triangle data to the vertex buffer.
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	if(FAILED(m_pVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin))))
		return false;
	memcpy(pVertexDataBegin, vertices.data(), sizeof(vertexBufferSize));
	m_pVertexBuffer->Unmap(0, nullptr);

	m_VertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = sizeof(Vertex);
	m_VertexBufferView.SizeInBytes = vertexBufferSize;
    return true;
}

const ComPtr<ID3D12GraphicsCommandList>& RenderableDX::getBundle() const {
	return m_pBundle;
}

const ComPtr<ID3D12PipelineState>& RenderableDX::getPipelineState() const {
	return m_pPipelineState;
}
