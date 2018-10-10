#include "HelperDX.hpp"

#include "thirdparty/d3dx12/d3dx12.h"

std::pair<HRESULT, ComPtr<IDXGIFactory5>> HelperDX::createFactory(UINT flags) {
	IDXGIFactory5* pFactory;
	HRESULT result = CreateDXGIFactory2(flags, IID_PPV_ARGS(&pFactory));
	return std::make_pair(result, std::move(pFactory));
}

std::vector<ComPtr<IDXGIAdapter1>> HelperDX::getAdapters(IDXGIFactory5* pFactory) {
	std::vector<ComPtr<IDXGIAdapter1>> adapters;
	ComPtr<IDXGIAdapter1> pAdapter;
	for (UINT i = 0; pFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; i++)
		adapters.push_back(std::move(pAdapter));
	return adapters;
}

std::vector<ComPtr<IDXGIOutput>> HelperDX::getOutputsForAdapter(IDXGIAdapter1* pAdapter) {
	std::vector<ComPtr<IDXGIOutput>> outputs;
	ComPtr<IDXGIOutput> pOutput;
	for (UINT i = 0; pAdapter->EnumOutputs(i, &pOutput) != DXGI_ERROR_NOT_FOUND; i++)
		outputs.push_back(std::move(pOutput));
	return outputs;
}

std::vector<DXGI_MODE_DESC> HelperDX::getDisplayModesForOutput(IDXGIOutput* pOutput, DXGI_FORMAT format) {
	std::vector<DXGI_MODE_DESC> displayModes;
	UINT numModes;
	pOutput->GetDisplayModeList(format, 0, &numModes, nullptr);
	displayModes.resize(numModes);
	pOutput->GetDisplayModeList(format, 0, &numModes, displayModes.data());
	return displayModes;
}

std::pair<HRESULT, ComPtr<IDXGISwapChain1>> HelperDX::createSwapchain(IDXGIFactory5* pFactory, ID3D12CommandQueue* pCommandQueue, 
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
	HRESULT result = pFactory->CreateSwapChainForHwnd(pCommandQueue, hWnd, &swapchainDesc, &fullscreenDesc, pOutput, &pSwapchain);
	return std::make_pair(result, std::move(pSwapchain));
}

std::pair<HRESULT, ComPtr<ID3D12Device>> HelperDX::createDevice(IDXGIAdapter1* pAdapter) {
	ComPtr<ID3D12Device> pDevice;
	HRESULT result = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice));
	return std::make_pair(result, std::move(pDevice));
}

std::pair<HRESULT, ComPtr<ID3D12CommandQueue>> HelperDX::createCommandQueue(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type) {
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = type;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	ComPtr<ID3D12CommandQueue> pCommandQueue;
	HRESULT result = pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&pCommandQueue));
	return std::make_pair(result, std::move(pCommandQueue));
}

std::pair<HRESULT, ComPtr<ID3D12CommandAllocator>> HelperDX::createCommandAllocator(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type) {
	ComPtr<ID3D12CommandAllocator> pCommandAllocator;
	HRESULT result = pDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&pCommandAllocator));
	return std::make_pair(result, std::move(pCommandAllocator));
}

std::pair<HRESULT, ComPtr<ID3D12GraphicsCommandList>> HelperDX::createCommandList(ID3D12Device* pDevice, ID3D12CommandAllocator* pCommandAllocator, D3D12_COMMAND_LIST_TYPE type) {
	ComPtr<ID3D12GraphicsCommandList> pCommandList;
	HRESULT result = pDevice->CreateCommandList(0, type, pCommandAllocator, nullptr, IID_PPV_ARGS(&pCommandList));
	return std::make_pair(result, std::move(pCommandList));
}

std::pair<HRESULT, ComPtr<ID3D12RootSignature>> HelperDX::createRootSignature(ID3D12Device* pDevice) {
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> pSignature;
	ComPtr<ID3DBlob> pError;
	ComPtr<ID3D12RootSignature> pRootSignature;
	HRESULT serializeResult = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError);
	if (FAILED(serializeResult))
		return std::make_pair(serializeResult, nullptr);
	HRESULT result = pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&pRootSignature));
	return std::make_pair(result, std::move(pRootSignature));
}
