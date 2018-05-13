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

#include "DriverD3D.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

DriverD3D::DriverD3D(GLFWwindow* pWindow) : IDriver(pWindow) {}

bool DriverD3D::initialize() {
#ifdef _DEBUG
	if(FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_pDebug))))
		return false;
#endif
	if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(&m_pFactory))))
		return false;

	IDXGIAdapter1* pAdapter;
	for (UINT i = 0; m_pFactory->EnumAdapters1(i, (&pAdapter)) 
		!= DXGI_ERROR_NOT_FOUND; i++) {
		DXGI_ADAPTER_DESC1 adapterDesc;
		pAdapter->GetDesc1(&adapterDesc);
		m_pAdapters.push_back(std::move(pAdapter));
	}
	pAdapter->Release();
	return true;
}

std::vector<Gpu> DriverD3D::getGpus() {
	return std::vector<Gpu>();
}

bool DriverD3D::selectGpu(uint8_t id) {
	if (FAILED(D3D12CreateDevice(m_pAdapters[id].Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice))))
		return false;

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapchainFullscreenDesc;

	if (FAILED(m_pFactory->CreateSwapChainForHwnd(m_pDevice.Get(), glfwGetWin32Window(getWindow()),
		&swapchainDesc, &swapchainFullscreenDesc, nullptr,
		reinterpret_cast<IDXGISwapChain1**>(m_pSwapchain.GetAddressOf()))))
		return false;
	return true;
}
