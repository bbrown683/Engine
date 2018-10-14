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

#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include "renderer/driver.hpp"
#include "thirdparty/glm/glm.hpp"

class RenderableDX;
class DriverDX : public Driver {
public:
    DriverDX(const SDL_Window* pWindow);
	~DriverDX();

    // Inherited via IDriver
    bool initialize() override;
    bool selectGpu(uint32_t id) override;
	bool prepareFrame() override;
    bool presentFrame() override;
    std::unique_ptr<Renderable> createRenderable() override;
	void addRenderable(Renderable* renderable) override;
    const ComPtr<ID3D12Device>& getDevice() const;
	const ComPtr<ID3D12GraphicsCommandList>& getCommandList() const;
	const ComPtr<ID3D12CommandAllocator>& getCommandAllocator() const;
	const ComPtr<ID3D12CommandAllocator>& getBundledAllocator() const;
	const ComPtr<ID3D12RootSignature>& getRootSignature() const;
private:
#ifdef _DEBUG
	ComPtr<IDXGIDebug1> m_pDxgiDebug;
	ComPtr<ID3D12Debug1> m_pDebug;
#endif
	ComPtr<IDXGIFactory5> m_pFactory;
    ComPtr<ID3D12Device> m_pDevice;
	std::vector<ComPtr<IDXGIAdapter1>> m_pAdapters;
	ComPtr<IDXGISwapChain3> m_pSwapchain;
    ComPtr<ID3D12CommandQueue> m_pCommandQueue;
	ComPtr<ID3D12CommandAllocator> m_pCommandAllocator;
	ComPtr<ID3D12CommandAllocator> m_pBundleAllocator;
	ComPtr<ID3D12RootSignature> m_pRootSignature;
	ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap;
	std::vector<ComPtr<ID3D12Resource>> m_pRenderTargets;
	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
	ComPtr<ID3D12Fence> m_pFence;
	std::vector<RenderableDX*> m_pRenderables;
	HANDLE m_pFenceEvent;
	UINT m_FenceValue;
	UINT m_FrameIndex; 
	UINT m_HeapSize;
	UINT m_RenderTargetCount;
	glm::vec4 m_ClearColor;
};