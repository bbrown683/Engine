#pragma once

#include <optional>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

struct HelperDX {
public:
	static std::optional<ComPtr<IDXGIFactory5>> createFactory(UINT flags);
	static std::vector<ComPtr<IDXGIAdapter1>> getAdapters(IDXGIFactory5* pFactory);
	static std::vector<ComPtr<IDXGIOutput>> getOutputsForAdapter(IDXGIAdapter1* pAdapter);
	static std::vector<DXGI_MODE_DESC> getDisplayModesForOutput(IDXGIOutput* pOutput, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	static std::optional<ComPtr<IDXGISwapChain1>> createSwapchain(IDXGIFactory5* pFactory, ID3D12CommandQueue* pCommandQueue,
		HWND hWnd, UINT renderTargets, BOOL windowed = true, IDXGIOutput* pOutput = nullptr, DXGI_MODE_DESC mode = {});
	static std::optional<ComPtr<ID3D12Device>> createDevice(IDXGIAdapter1* pAdapter);
	static std::optional<ComPtr<ID3D12CommandQueue>> createCommandQueue(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
	static std::optional<ComPtr<ID3D12CommandAllocator>> createCommandAllocator(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
	static std::optional<ComPtr<ID3D12GraphicsCommandList>> createCommandList(ID3D12Device* pDevice, ID3D12CommandAllocator* pCommandAllocator, 
		D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
	static std::optional<ComPtr<ID3D12RootSignature>> createRootSignature(ID3D12Device* pDevice);
};