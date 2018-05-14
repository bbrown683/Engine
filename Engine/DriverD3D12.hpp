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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <wrl/client.h>
using namespace Microsoft::WRL;

#include "Driver.hpp"

class DriverD3D12 : public Driver {
public:
    DriverD3D12(GLFWwindow* pWindow);

    // Inherited via IDriver
    bool initialize() override;
    bool selectGpu(uint8_t id) override;
    void submit() override;
    std::unique_ptr<Renderable> createRenderable(bool once) override;
private:
#ifdef _DEBUG
    ComPtr<IDXGIDebug1> m_pCpuDebug;
    ComPtr<ID3D12Debug1> m_pGpuDebug;
#endif
    ComPtr<ID3D12Device> m_pDevice;
    ComPtr<ID3D12CommandQueue> m_pCommandQueue;
    ComPtr<ID3D12Fence> m_pFence;
    ComPtr<IDXGIFactory5> m_pFactory;
    std::vector<ComPtr<IDXGIAdapter1>> m_pAdapters;
    ComPtr<IDXGISwapChain1> m_pSwapchain;
};