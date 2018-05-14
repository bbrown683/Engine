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

#include <iostream>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

DriverD3D12::DriverD3D12(GLFWwindow* pWindow) : Driver(pWindow) {}

bool DriverD3D12::initialize() {
#ifdef _DEBUG
    if (FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_pCpuDebug))))
        return false;
    m_pCpuDebug->EnableLeakTrackingForThread();
    m_pCpuDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);

    if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_pGpuDebug))))
        return false;
    m_pGpuDebug->EnableDebugLayer();
    m_pGpuDebug->SetEnableGPUBasedValidation(true);
#endif
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&m_pFactory))))
        return false;

    IDXGIAdapter1* pAdapter;
    for (UINT i = 0; m_pFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
        DXGI_ADAPTER_DESC1 adapterDesc;
        pAdapter->GetDesc1(&adapterDesc);

        Gpu gpu;
        gpu.id = static_cast<uint8_t>(i);
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

bool DriverD3D12::selectGpu(uint8_t id) {
    // id Does not correlate to a proper GPU.
    if (id >= m_pAdapters.size())
        return false;

    if (FAILED(D3D12CreateDevice(m_pAdapters[id].Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pDevice))))
        return false;

    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    if (FAILED(m_pDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_pCommandQueue))))
        return false;

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
    swapchainDesc.BufferCount = 2;
    swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.SampleDesc.Count = 1;

    if (FAILED(m_pFactory->CreateSwapChainForHwnd(m_pCommandQueue.Get(), glfwGetWin32Window(getWindow()),
        &swapchainDesc, nullptr, nullptr, m_pSwapchain.GetAddressOf())))
        return false;
    return true;
}

void DriverD3D12::beginFrame() {}

void DriverD3D12::endFrame() {}
