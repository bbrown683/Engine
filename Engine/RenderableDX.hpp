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

#include <vector>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include "Renderable.hpp"

class DriverDX;
class RenderableDX : public Renderable {
public:
    RenderableDX(DriverDX* pDriver);
    bool build() override;
    bool attachShader(const char* pFilename, ShaderStage stage) override;
    bool setIndices(std::vector<uint16_t> indices) override;
    bool setVertices(std::vector<Vertex> vertices) override;
	const ComPtr<ID3D12GraphicsCommandList>& getBundle() const;
	const ComPtr<ID3D12PipelineState>& getPipelineState() const;
private:
    DriverDX* m_pDriver;
	ComPtr<ID3D12GraphicsCommandList> m_pBundle;
	ComPtr<ID3D12PipelineState> m_pPipelineState;
	ComPtr<ID3DBlob> m_pVertexShader;
	ComPtr<ID3DBlob> m_pPixelShader; 
	ComPtr<ID3D12Resource> m_pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
	std::vector<uint16_t> m_Indices;
	std::vector<std::vector<float>> m_Vertices;
};