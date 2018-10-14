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

#include "driver_dx12.hpp"

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include "thirdparty/d3dx12/d3dx12.h"
#include "thirdparty/loguru/loguru.hpp"
#include "thirdparty/glm/gtc/type_ptr.hpp"

#include "helper_dx12.hpp"

DriverDX12::DriverDX12(const SDL_Window* pWindow) : Driver(pWindow) {
	m_FrameIndex = 0;
	m_HeapSize = 0;
	m_RenderTargetCount = 2;
	m_pRenderTargets = std::vector<ComPtr<ID3D12Resource>>(m_RenderTargetCount);
	m_ClearColor = { 0.0f, 0.2f, 0.4f, 1.0f };
}

DriverDX12::~DriverDX12() {
	CloseHandle(m_pFenceEvent);
	LOG_F(INFO, "DirectX 12 driver shutting down.");
}

bool DriverDX12::initialize() {
	LOG_F(INFO, "DirectX 12 driver initializing.");
#ifdef _DEBUG
	if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_pDebug))))
		return false;
	m_pDebug->EnableDebugLayer();

	if (FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_pDxgiDebug))))
		return false;
	m_pDxgiDebug->EnableLeakTrackingForThread();
#endif
	
	if(auto factory = HelperDX12::createFactory(0); factory.has_value())
		m_pFactory.Swap(factory.value());
	else {
		LOG_F(FATAL, "Failed to create DXGI Factory.");
		return false;
	}

	LOG_F(INFO, "Enumerating Adapters:");
    // Enumerate each GPU or software rasterizer found on the system.
    // These will be used to select a GPU to render with
	m_pAdapters = HelperDX12::getAdapters(m_pFactory.Get());
	if (m_pAdapters.empty())
		return false;
	for (size_t i = 0; i < m_pAdapters.size(); i++) {
		DXGI_ADAPTER_DESC1 adapterDesc;
		m_pAdapters[i]->GetDesc1(&adapterDesc);
		// Skip software implementations.
		if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		Gpu gpu;
		gpu.id = static_cast<uint32_t>(i);
		gpu.vendorId = adapterDesc.VendorId;
		gpu.deviceId = adapterDesc.DeviceId;
		gpu.memory = adapterDesc.DedicatedVideoMemory / 1024 / 1024;
		size_t length = std::wcstombs(gpu.name, adapterDesc.Description, 256);
		if (length != -1)
			gpu.name[length] = '\0';
		LOG_F(INFO, "\t[%u]: %s", gpu.id, gpu.name);
		LOG_F(INFO, "\t\tVideoMemory: %uMB ", gpu.memory);
		LOG_F(INFO, "\t\tVendorId: %u", gpu.vendorId);
		LOG_F(INFO, "\t\tDeviceId: %u", gpu.deviceId);
		addGpu(gpu);
    }
    return true;
}

bool DriverDX12::selectGpu(uint32_t id) {
    // id Does not correlate to a proper GPU.
    if (id >= m_pAdapters.size())
        return false;

	IDXGIAdapter1* adapter = m_pAdapters[id].Get();

	// Create the main rendering device.
	if (auto device = HelperDX12::createDevice(adapter); device.has_value())
		m_pDevice.Swap(device.value());
	else {
		LOG_F(FATAL, "Failed to create device.");
		return false;
	}

	// Create the command queue to submit our command lists to.
	if (auto commandQueue = HelperDX12::createCommandQueue(m_pDevice.Get()); commandQueue.has_value())
		m_pCommandQueue.Swap(commandQueue.value());
	else {
		LOG_F(FATAL, "Failed to create command queue");
		return false;
	}

	// Create the command allocator for our primary command list to submit to.
	if (auto commandAllocator = HelperDX12::createCommandAllocator(m_pDevice.Get()); commandAllocator.has_value())
		m_pCommandAllocator.Swap(commandAllocator.value());
	else {
		LOG_F(FATAL, "Failed to create command allocator.");
		return false;
	}

	// Create the command allocator for our primary command list to submit to.
	if (auto bundleAllocator = HelperDX12::createCommandAllocator(m_pDevice.Get(), D3D12_COMMAND_LIST_TYPE_BUNDLE); bundleAllocator.has_value())
		m_pBundleAllocator.Swap(bundleAllocator.value());
	else {
		LOG_F(FATAL, "Failed to create bundle allocator.");
		return false;
	}

	if (auto commandList = HelperDX12::createCommandList(m_pDevice.Get(), m_pCommandAllocator.Get()); commandList.has_value())
		m_pCommandList.Swap(commandList.value());
	else {
		LOG_F(FATAL, "Failed to create primary command list.");
		return false;
	}

	if (FAILED(m_pCommandList->Close()))
		return false;

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(const_cast<SDL_Window*>(getWindow()), &wmInfo))
        return false;
	HWND hWnd = wmInfo.info.win.window;

	if (auto swapchain = HelperDX12::createSwapchain(m_pFactory.Get(), m_pCommandQueue.Get(), hWnd, m_RenderTargetCount); swapchain.has_value())
		swapchain.value().As(&m_pSwapchain);
	else {
		LOG_F(FATAL, "Failed to create DXGI Swapchain");
		return false;
	}

	m_FrameIndex = m_pSwapchain->GetCurrentBackBufferIndex();

	if (auto rootSignature = HelperDX12::createRootSignature(m_pDevice.Get()); rootSignature.has_value())
		m_pRootSignature.Swap(rootSignature.value());
	else {
		LOG_F(FATAL, "Failed to create root signature");
		return false;
	}

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = m_RenderTargetCount;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(m_pDevice->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_pDescriptorHeap))))
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
	LOG_F(INFO, "DirectX 12 driver was successfully initialized.");
	return true;
}

bool DriverDX12::prepareFrame() {
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

	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT));

	if(FAILED(m_pCommandList->Close()))
		return false;
	return true;
}

bool DriverDX12::presentFrame() {
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

const ComPtr<ID3D12Device>& DriverDX12::getDevice() const {
    return m_pDevice;
}

const ComPtr<ID3D12GraphicsCommandList>& DriverDX12::getCommandList() const {
	return m_pCommandList;
}

const ComPtr<ID3D12CommandAllocator>& DriverDX12::getCommandAllocator() const {
	return m_pCommandAllocator;
}

const ComPtr<ID3D12CommandAllocator>& DriverDX12::getBundledAllocator() const {
	return m_pBundleAllocator;
}

const ComPtr<ID3D12RootSignature>& DriverDX12::getRootSignature() const {
	return m_pRootSignature;
}
