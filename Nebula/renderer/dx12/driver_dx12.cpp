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
	m_renderTargetHeapSize = 0;
	m_RenderTargetCount = 2;
	m_pRenderTargets = std::vector<ComPtr<ID3D12Resource>>(m_RenderTargetCount);
	m_pCommandAllocators = std::vector<ComPtr<ID3D12CommandAllocator>>(m_RenderTargetCount);
	m_FenceValues = std::vector<UINT64>(m_RenderTargetCount);
	m_ClearColor = { 0.0f, 0.2f, 0.4f, 1.0f };

	// Set viewport and scissor rects.
	int width, height;
	SDL_GetWindowSize(const_cast<SDL_Window*>(getWindow()), &width, &height);
	m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
	m_ScissorRect = CD3DX12_RECT(0, 0, static_cast<long>(width), static_cast<long>(height));

	aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

DriverDX12::~DriverDX12() {
	waitOnFence();
	CloseHandle(m_pFenceEvent);
	LOG_F(INFO, "DirectX 12 driver shutting down.");
}

bool DriverDX12::initialize() {
	LOG_F(INFO, "DirectX 12 driver initializing.");
#ifdef _DEBUG
	if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_pDebug))))
		return false;
	m_pDebug->EnableDebugLayer();
	LOG_F(INFO, "DirectX 12 debug layer initialized.");
#endif
	
	// Create the factory which will be used for our swapchain later.
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&m_pFactory))))
		return false;

	LOG_F(INFO, "Enumerating Adapters:");
    // Enumerate each GPU or software rasterizer found on the system.
    // These will be used to select a GPU to render with
	ComPtr<IDXGIAdapter1> pAdapter;
	for (UINT i = 0; m_pFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
		DXGI_ADAPTER_DESC1 adapterDesc;
		pAdapter->GetDesc1(&adapterDesc);
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
		m_pAdapters.push_back(std::move(pAdapter));
		addGpu(gpu);
    }
    return !m_pAdapters.empty();
}

bool DriverDX12::selectGpu(uint32_t id) {
    // id Does not correlate to a proper GPU.
    if (id >= m_pAdapters.size())
        return false;

	IDXGIAdapter1* adapter = m_pAdapters[id].Get();

	// Create the device which is attached to the GPU.
	if (FAILED(D3D12CreateDevice(m_pAdapters[id].Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice)))) {
		LOG_F(FATAL, "Could not create D3D12 rendering device.");
		return false;
	}

	// Create a queue for passing our command lists to.
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	if (FAILED(m_pDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_pCommandQueue)))) {
		LOG_F(FATAL, "Could not create D3D12 command queue.");
		return false;
	}

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

	// Initialize and create Render target view heap.
	D3D12_DESCRIPTOR_HEAP_DESC renderTargetHeapDesc = {};
	renderTargetHeapDesc.NumDescriptors = m_RenderTargetCount;
	renderTargetHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	if (FAILED(m_pDevice->CreateDescriptorHeap(&renderTargetHeapDesc, IID_PPV_ARGS(&m_pRenderTargetHeap))))
		return false;
	m_renderTargetHeapSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Configure the descriptor for CPU memory data.
	CD3DX12_CPU_DESCRIPTOR_HANDLE renderTargetDescriptor(m_pRenderTargetHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < m_RenderTargetCount; i++) {
		if (FAILED(m_pSwapchain->GetBuffer(i, IID_PPV_ARGS(&m_pRenderTargets[i]))))
			return false;
		m_pDevice->CreateRenderTargetView(m_pRenderTargets[i].Get(), nullptr, renderTargetDescriptor);
		renderTargetDescriptor.Offset(1, m_renderTargetHeapSize);
		if (FAILED(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocators[i]))))
			return false;
	}

	build();
	LOG_F(INFO, "DirectX 12 driver was successfully initialized.");
	return true;
}

bool DriverDX12::prepareFrame() {
	m_pCommandAllocators[m_FrameIndex]->Reset();
	m_pCommandList->Reset(m_pCommandAllocators[m_FrameIndex].Get(), m_pPipelineState.Get());

	// Set typical command list state.
	m_pCommandList->SetGraphicsRootSignature(m_pRootSignature.Get());
	m_pCommandList->RSSetViewports(1, &m_Viewport);
	m_pCommandList->RSSetScissorRects(1, &m_ScissorRect);

	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET));
	// Grab handle to our descriptor.
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_pRenderTargetHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, m_renderTargetHeapSize);
	m_pCommandList->OMSetRenderTargets(1, &descriptorHandle, false, nullptr);
	m_pCommandList->ClearRenderTargetView(descriptorHandle, glm::value_ptr(m_ClearColor), 0, nullptr);

	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pCommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	m_pCommandList->DrawInstanced(3, 1, 0, 0);

	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT));

	m_pCommandList->Close();
	return true;
}

bool DriverDX12::presentFrame() {
	std::array<ID3D12CommandList*, 1> commandLists = { m_pCommandList.Get() };
	m_pCommandQueue->ExecuteCommandLists(commandLists.size(), commandLists.data());

	m_pSwapchain->Present(1, 0);

	nextFrame();
	return true;
}

void DriverDX12::waitOnFence() {
	// Schedule a Signal command in the queue.
	m_pCommandQueue->Signal(m_pFence.Get(), m_FenceValues[m_FrameIndex]);

	// Wait until the fence has been processed.
	m_pFence->SetEventOnCompletion(m_FenceValues[m_FrameIndex], m_pFenceEvent);
	WaitForSingleObjectEx(m_pFenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	m_FenceValues[m_FrameIndex]++;
}

void DriverDX12::nextFrame() {
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = m_FenceValues[m_FrameIndex];
	m_pCommandQueue->Signal(m_pFence.Get(), currentFenceValue);

	// Update the frame index.
	m_FrameIndex = m_pSwapchain->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (m_pFence->GetCompletedValue() < m_FenceValues[m_FrameIndex]) {
		m_pFence->SetEventOnCompletion(m_FenceValues[m_FrameIndex], m_pFenceEvent);
		WaitForSingleObjectEx(m_pFenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	m_FenceValues[m_FrameIndex] = currentFenceValue + 1;
}

void DriverDX12::build() {
	if (auto rootSignature = HelperDX12::createRootSignature(m_pDevice.Get()); rootSignature.has_value())
		m_pRootSignature.Swap(rootSignature.value());
	else {
		LOG_F(FATAL, "Failed to create root signature");
		return;
	}

	auto path = L"C:\\Users\\Ben\\nebula\\nebula\\shaders\\shaders.hlsl";
	HRESULT vert = D3DCompileFromFile(path, nullptr, nullptr, "VSMain", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_pVertexShader, nullptr);
	HRESULT frag = D3DCompileFromFile(path, nullptr, nullptr, "PSMain", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_pPixelShader, nullptr);

	// Define the vertex input layout.
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs.data(), inputElementDescs.size() };
	psoDesc.pRootSignature = m_pRootSignature.Get();
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
	m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState));

	m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocators[m_FrameIndex].Get(), m_pPipelineState.Get(), IID_PPV_ARGS(&m_pCommandList));
	m_pCommandList->Close();

	VERTEX vertices[] = {
			{ { 0.0f, 0.25f * aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f * aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f * aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
	};

	UINT vertexBufferSize = sizeof(vertices);

	m_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pVertexBuffer));

	// Copy the triangle data to the vertex buffer.
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	m_pVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
	memcpy(pVertexDataBegin, vertices, sizeof(vertexBufferSize));
	m_pVertexBuffer->Unmap(0, nullptr);

	m_VertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = sizeof(Vertex);
	m_VertexBufferView.SizeInBytes = vertexBufferSize;

	m_pDevice->CreateFence(m_FenceValues[m_FrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
	m_FenceValues[m_FrameIndex]++;

	// Create an event handle to use for frame synchronization.
	m_pFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_pFenceEvent == nullptr) {
		HRESULT_FROM_WIN32(GetLastError());
	}

	// Wait for the command list to execute; we are reusing the same command 
	// list in our main loop but for now, we just want to wait for setup to 
	// complete before continuing.
	waitOnFence();
}

const ComPtr<ID3D12Device>& DriverDX12::getDevice() const {
    return m_pDevice;
}

const ComPtr<ID3D12GraphicsCommandList>& DriverDX12::getCommandList() const {
	return m_pCommandList;
}

const ComPtr<ID3D12CommandAllocator>& DriverDX12::getBundledAllocator() const {
	return m_pBundleAllocator;
}

const ComPtr<ID3D12RootSignature>& DriverDX12::getRootSignature() const {
	return m_pRootSignature;
}
