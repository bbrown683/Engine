#include "helper_dx12.hpp"

#include "thirdparty/d3dx12/d3dx12.h"

std::optional<ComPtr<IDXGISwapChain1>> HelperDX12::createSwapchain(IDXGIFactory5* pFactory, ID3D12CommandQueue* pCommandQueue, 
	HWND hWnd, UINT renderTargets, BOOL windowed, IDXGIOutput* pOutput, DXGI_MODE_DESC mode) {
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.BufferCount = renderTargets;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.Width = mode.Width;
	swapchainDesc.Height = mode.Height;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = {};
	fullscreenDesc.RefreshRate = mode.RefreshRate;
	fullscreenDesc.Scaling = mode.Scaling;
	fullscreenDesc.ScanlineOrdering = mode.ScanlineOrdering;
	fullscreenDesc.Windowed = windowed;

	ComPtr<IDXGISwapChain1> pSwapchain;
	if (FAILED(pFactory->CreateSwapChainForHwnd(pCommandQueue, hWnd, &swapchainDesc, &fullscreenDesc, pOutput, &pSwapchain)))
		return {};
	return pSwapchain;
}

std::optional<ComPtr<ID3D12RootSignature>> HelperDX12::createRootSignature(ID3D12Device* pDevice) {
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> pSignature;
	ComPtr<ID3DBlob> pError;
	ComPtr<ID3D12RootSignature> pRootSignature;
	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError)))
		return {};
	if (FAILED(pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&pRootSignature))))
		return {};
	return pRootSignature;
}
