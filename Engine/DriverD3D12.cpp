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

#include "DriverD3D12.hpp"
#include "RenderableD3D12.hpp"

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

DriverD3D12::DriverD3D12(const SDL_Window* pWindow) : Driver(pWindow) {}

bool DriverD3D12::initialize() {
    // Enable debug layer for D3D12 and DXGI.
#ifdef _DEBUG
    if (FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_pCpuDebug))))
        return false;
    m_pCpuDebug->EnableLeakTrackingForThread();
    m_pCpuDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);

    if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_pGpuDebug))))
        return false;
    //m_pGpuDebug->EnableDebugLayer();
    //m_pGpuDebug->SetEnableGPUBasedValidation(true);
#endif
    // Create the factory which will be used for our swapchain later.
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&m_pFactory))))
        return false;

    // Enumerate each GPU or software rasterizer found on the system.
    // These will be used to select a GPU to render with.
    IDXGIAdapter1* pAdapter;
    for (UINT i = 0; m_pFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
        DXGI_ADAPTER_DESC1 adapterDesc;
        pAdapter->GetDesc1(&adapterDesc);

        Gpu gpu;
        gpu.id = static_cast<uint32_t>(i);
        gpu.memory = static_cast<uint32_t>(adapterDesc.DedicatedVideoMemory);
        gpu.software = adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE ? true : false;
        size_t length = std::wcstombs(gpu.name, adapterDesc.Description, 256);
        if (length != -1)
            gpu.name[length] = '\0';
        addGpu(gpu);
        m_pAdapters.push_back(std::move(pAdapter));
    }
    return !m_pAdapters.empty();
}

bool DriverD3D12::selectGpu(uint32_t id) {
    // id Does not correlate to a proper GPU.
    if (id >= m_pAdapters.size())
        return false;

    // Create the device which is attached to the GPU.
    if (FAILED(D3D12CreateDevice(m_pAdapters[id].Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pDevice))))
        return false;

    // Create a queue for passing our command lists to.
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    if (FAILED(m_pDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_pPrimaryCommandQueue))))
        return false;

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc {};
    swapchainDesc.BufferCount = 3;
    swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.SampleDesc.Count = 1;

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(const_cast<SDL_Window*>(getWindow()), &wmInfo))
        return false;

    if (FAILED(m_pFactory->CreateSwapChainForHwnd(m_pPrimaryCommandQueue.Get(), wmInfo.info.win.window,
        &swapchainDesc, nullptr, nullptr, m_pSwapchain.GetAddressOf())))
        return false;
    return true;
}

bool DriverD3D12::presentFrame() {
    /*
    if (FAILED(m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence))))
        return false;

    // Execute the primary command list.
    m_pPrimaryCommandQueue->ExecuteCommandLists(1, &m_pPrimaryCommandList);
 
    // Wait for command queue to complete submission to GPU.
    m_pPrimaryCommandQueue->Wait(m_pFence.Get(), UINT64_MAX);
    if (FAILED(m_pSwapchain->Present1(1, 0, nullptr)))
        return false;
    m_pFence.Reset();
    */
    m_pSwapchain->Present(0, 0);
    return true;
}

std::unique_ptr<Renderable> DriverD3D12::createRenderable() {
    return std::make_unique<RenderableD3D12>(this);
}

const ComPtr<ID3D12Device>& DriverD3D12::getDevice() const {
    return m_pDevice;
}

const ComPtr<ID3D12CommandList>& DriverD3D12::getPrimaryCommandList() const {
    return m_pPrimaryCommandList;
}

const ComPtr<ID3DBlob>& DriverD3D12::getBlobFromCache(const char* pFilename) {
    auto iter = m_pBlobCache.find(pFilename);
    return iter->second;
}
