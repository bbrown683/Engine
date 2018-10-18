#pragma once

#include <optional>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

struct HelperDX12 {
public:
	static std::optional<ComPtr<IDXGISwapChain1>> createSwapchain(IDXGIFactory5* pFactory, ID3D12CommandQueue* pCommandQueue,
		HWND hWnd, UINT renderTargets, BOOL windowed = true, IDXGIOutput* pOutput = nullptr, DXGI_MODE_DESC mode = {});
	static std::optional<ComPtr<ID3D12RootSignature>> createRootSignature(ID3D12Device* pDevice);
};