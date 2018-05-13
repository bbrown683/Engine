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
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
using namespace Microsoft::WRL;

#include "IDriver.hpp"

class DriverD3D : public IDriver {
public:
	DriverD3D(GLFWwindow* pWindow);

	// Inherited via IDriver
	bool initialize() override;
	std::vector<Gpu> getGpus() override;
	bool selectGpu(uint8_t id) override;
private:
	ComPtr<ID3D12Debug2> m_pDebug;
	ComPtr<ID3D12Device> m_pDevice;
	ComPtr<IDXGIFactory5> m_pFactory;
	std::vector<ComPtr<IDXGIAdapter1>> m_pAdapters;
	ComPtr<IDXGISwapChain4> m_pSwapchain;
};