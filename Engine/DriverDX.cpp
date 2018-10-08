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

#include "DriverDX.hpp"

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include "thirdparty/d3dx12/d3dx12.h"
#include "thirdparty/loguru/loguru.hpp"
#include "thirdparty/glm/gtc/type_ptr.hpp"

#include "RenderableDX.hpp"

DriverDX::DriverDX(const SDL_Window* pWindow) : Driver(pWindow) {
	m_FrameIndex = 0;
	m_HeapSize = 0;
	m_RenderTargetCount = 2;
	m_pRenderTargets = std::vector<ComPtr<ID3D12Resource>>(m_RenderTargetCount);
	m_ClearColor = { 0.0f, 0.2f, 0.4f, 1.0f };
}

DriverDX::~DriverDX() {
	CloseHandle(m_pFenceEvent);
#ifdef _DEBUG
	//m_pDxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
#endif
}

bool DriverDX::initialize() {
#ifdef _DEBUG
	if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_pDebug))))
		return false;
	m_pDebug->EnableDebugLayer();
	//m_pDebug->SetEnableGPUBasedValidation(true);
	//m_pDebug->SetEnableSynchronizedCommandQueueValidation(true);

	if (FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_pDxgiDebug))))
		return false;
	m_pDxgiDebug->EnableLeakTrackingForThread();
#endif
	// Create the factory which will be used for our swapchain later.
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&m_pFactory))))
		return false;

	LOG_F(INFO, "Enumerating Adapters:");
    // Enumerate each GPU or software rasterizer found on the system.
    // These will be used to select a GPU to render with.
    ComPtr<IDXGIAdapter1> pAdapter;
    for (UINT i = 0; m_pFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
        DXGI_ADAPTER_DESC1 adapterDesc;
		pAdapter->GetDesc1(&adapterDesc);

		Gpu gpu;
		gpu.id = static_cast<uint32_t>(i);
		gpu.vendorId = adapterDesc.VendorId;
		gpu.deviceId = adapterDesc.DeviceId;
		gpu.memory = static_cast<uint32_t>(adapterDesc.DedicatedVideoMemory / 1024 / 1024);
		gpu.software = adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE ? true : false;
		size_t length = std::wcstombs(gpu.name, adapterDesc.Description, 256);
		if (length != -1)
			gpu.name[length] = '\0';
		LOG_F(INFO, "\t[%u]: %s", gpu.id, gpu.name);
		LOG_F(INFO, "\t\tVideoMemory: %uMB ", gpu.memory);
		LOG_F(INFO, "\t\tVendorId: %u", gpu.vendorId);
		LOG_F(INFO, "\t\tDeviceId: %u", gpu.deviceId);
		addGpu(gpu);
		m_pAdapters.push_back(std::move(pAdapter));
    }
    return !m_pAdapters.empty();
}

bool DriverDX::selectGpu(uint32_t id) {
    // id Does not correlate to a proper GPU.
    if (id >= m_pAdapters.size())
        return false;

    // Create the device which is attached to the GPU.
	if (FAILED(D3D12CreateDevice(m_pAdapters[id].Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice)))) {
		LOG_F(FATAL, "Could not create D3D12 rendering device.");
		return false;
	}

	// Create a queue for passing our command lists to.
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc {};
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	if (FAILED(m_pDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_pCommandQueue)))) {
		LOG_F(FATAL, "Could not create D3D12 command queue.");
		return false;
	}

	// Create a queue to allocate our primary command list.
	if (FAILED(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator)))) {
		LOG_F(FATAL, "Could not create D3D12 command allocator.");
		return false;
	}

	// Create a queue to allocate our bundled command lists.
	if (FAILED(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&m_pBundleAllocator)))) {
		LOG_F(FATAL, "Could not create D3D12 bundle allocator.");
		return false;
	}

	// Create primary command list.
	if (FAILED(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pCommandList))))
		return false;
	
	if (FAILED(m_pCommandList->Close()))
		return false;

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc {};
    swapchainDesc.BufferCount = m_RenderTargetCount;
    swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.SampleDesc.Count = 1;

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(const_cast<SDL_Window*>(getWindow()), &wmInfo))
        return false;

	ComPtr<IDXGISwapChain1> pSwapchain;
    if (FAILED(m_pFactory->CreateSwapChainForHwnd(m_pCommandQueue.Get(), wmInfo.info.win.window,
        &swapchainDesc, nullptr, nullptr, pSwapchain.GetAddressOf())))
        return false;
	pSwapchain.As(&m_pSwapchain);
	m_FrameIndex = m_pSwapchain->GetCurrentBackBufferIndex();

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc {};
	descriptorHeapDesc.NumDescriptors = m_RenderTargetCount;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if(FAILED(m_pDevice->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_pDescriptorHeap))))
		return false;
	m_HeapSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Configure the descriptor for CPU memory data.
	CD3DX12_CPU_DESCRIPTOR_HANDLE destDescriptor(m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < m_RenderTargetCount; i++) {
		if (FAILED(m_pSwapchain->GetBuffer(i, IID_PPV_ARGS(&m_pRenderTargets[i]))))
			return false;
		m_pDevice->CreateRenderTargetView(m_pRenderTargets[i].Get(), nullptr, destDescriptor);
		destDescriptor.Offset(1, m_HeapSize);
	}

	// Initialize the root signature.
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc {};
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	
	// Compile and create root signature.
	ComPtr<ID3DBlob> pSignature;
	ComPtr<ID3DBlob> pError;
	if(FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pSignature, &pError)))
		return false;
	if(FAILED(m_pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature))))
		return false;  

	// Set viewport and scissor rects.
	int width, height;
	SDL_GetWindowSize(const_cast<SDL_Window*>(getWindow()), &width, &height);
	m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
	m_ScissorRect = CD3DX12_RECT(0, 0, static_cast<long>(width), static_cast<long>(height));

	// Create the fence to wait for completion.
	if(FAILED(m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence))))
		return false;

	m_FenceValue = 1;
	// Create an event handle to use for frame synchronization.
	m_pFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_pFenceEvent == nullptr) {
		if (FAILED(HRESULT_FROM_WIN32(GetLastError())))
			return false;
	}
	LOG_F(INFO, "DirectX driver was successfully initialized.");
	return true;
}

bool DriverDX::prepareFrame() {
	if (FAILED(m_pCommandAllocator->Reset()))
		return false;
	if (FAILED(m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr)))
		return false;

	// Set typical command list state.
	m_pCommandList->SetGraphicsRootSignature(m_pRootSignature.Get());
	m_pCommandList->RSSetViewports(1, &m_Viewport);
	m_pCommandList->RSSetScissorRects(1, &m_ScissorRect);

	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Grab handle to our descriptor.
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, m_HeapSize);
	m_pCommandList->OMSetRenderTargets(1, &descriptorHandle, false, nullptr);
	m_pCommandList->ClearRenderTargetView(descriptorHandle, glm::value_ptr(m_ClearColor), 0, nullptr);

	// Execute bundles for each renderable.
	for (size_t i = 0; i < m_pRenderables.size(); i++)
		m_pCommandList->ExecuteBundle(m_pRenderables[i]->getBundle().Get());

	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT));

	if(FAILED(m_pCommandList->Close()))
		return false;
	return true;
}

bool DriverDX::presentFrame() {
	std::array<ID3D12CommandList*, 1> commandLists = { m_pCommandList.Get() };
	m_pCommandQueue->ExecuteCommandLists(commandLists.size(), commandLists.data());

	if (FAILED(m_pSwapchain->Present(1, 0)))
		return false;

	// Signal and increment the fence value.
	const UINT64 fenceValue = m_FenceValue;
	if (FAILED(m_pCommandQueue->Signal(m_pFence.Get(), fenceValue)))
		return false;
	m_FenceValue++;

	// Wait until the previous frame is finished.
	if (m_pFence->GetCompletedValue() < fenceValue) {
		if (FAILED(m_pFence->SetEventOnCompletion(fenceValue, m_pFenceEvent)))
			return false;
		WaitForSingleObject(m_pFenceEvent, INFINITE);
	}

	// Update frame index after we are done presenting.
	m_FrameIndex = m_pSwapchain->GetCurrentBackBufferIndex();
	return true;
}
	
std::unique_ptr<Renderable> DriverDX::createRenderable() {
    return std::make_unique<RenderableDX>(this);
}

void DriverDX::addRenderable(Renderable* renderable) {
	m_pRenderables.push_back(dynamic_cast<RenderableDX*>(renderable));
}

const ComPtr<ID3D12Device>& DriverDX::getDevice() const {
    return m_pDevice;
}

const ComPtr<ID3D12GraphicsCommandList>& DriverDX::getCommandList() const {
	return m_pCommandList;
}

const ComPtr<ID3D12CommandAllocator>& DriverDX::getCommandAllocator() const {
	return m_pCommandAllocator;
}

const ComPtr<ID3D12CommandAllocator>& DriverDX::getBundledAllocator() const {
	return m_pBundleAllocator;
}

const ComPtr<ID3D12RootSignature>& DriverDX::getRootSignature() const {
	return m_pRootSignature;
}
