#include "Windows.h"
#include "Window.h"
#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>
#include "Debug.h"
#include <vector>



HRESULT CreateDebugInterface(Microsoft::WRL::ComPtr<ID3D12Debug6>* debugInterface)
{
	Microsoft::WRL::ComPtr<ID3D12Debug6> debug;
	HRESULT result = D3D12GetDebugInterface(__uuidof(ID3D12Debug6), (LPVOID*)&debug);
	
	if (FAILED(result))
	{
		DEBUG("Failed creating debug interface");
	}
	else
	{
		debug.As(debugInterface);
	}

	return result;
}

HRESULT CreateFactory(UINT flag, Microsoft::WRL::ComPtr<IDXGIFactory6>* factory)
{
	Microsoft::WRL::ComPtr<IDXGIFactory6> fact;
	HRESULT result = CreateDXGIFactory2(flag, __uuidof(IDXGIFactory6), (LPVOID*)&fact);
	if (FAILED(result))
	{
		DEBUG("Failed creating factory");
	}
	else
	{
		fact.As(factory);
	}

	return result;

}

HRESULT GetDisplayAdapter(Microsoft::WRL::ComPtr<IDXGIAdapter4>* deviceAdapter, IDXGIFactory6* factory)
{
	HRESULT result;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

	for (UINT adapterIndex = 0; SUCCEEDED(factory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}
		result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(result))
		{
			break;
		}

	}

	adapter.As(deviceAdapter);

	return result;
}

HRESULT CreateDevice(IDXGIAdapter4* adapter, Microsoft::WRL::ComPtr<ID3D12Device10>* device)
{
	HRESULT result = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device10), (LPVOID*)device);
	if (FAILED(result))
	{
		DEBUG("Failed creating device");
	}

	device->Get()->SetName(L"MainDevice");
	DEBUG("Device Created");

	return result;
}

void DisplayAdapterInfo(IDXGIAdapter4* deviceAdapter)
{
	DXGI_ADAPTER_DESC3 desc;
	deviceAdapter->GetDesc3(&desc);
	DEBUG(desc.Description);
	DEBUG(desc.DedicatedVideoMemory);
}


int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Window window(L"DX12", 1920, 1080, hInstance);

	//Create factory
	//Create adapter
	//Create device
	//Command Queue
	//Create command allocator
	//Create command list
	//Create Swap chain
	//Create depth stencil buffer resource/view
	//Create base viewport
	//create a fence for command list
	//Root descriptors?
	//Pipeline state objects? (PSOs)

	//Create vertex and index buffers
	//Create vertex and pixel shaders
	//... 
	//Submit triangle draw call
	//Execute command list

	Microsoft::WRL::ComPtr<ID3D12Debug6> debug;
	Microsoft::WRL::ComPtr<IDXGIFactory6> factory;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter;
	Microsoft::WRL::ComPtr<ID3D12Device10> device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapchain;
	Microsoft::WRL::ComPtr<ID3D12Resource> frameBuffers[2];
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmdList;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	Microsoft::WRL::ComPtr<ID3D12Fence1> fence;
	UINT fenceValue;
	HANDLE fenceEvent;

	UINT rtvHeapOffset;
	UINT currentFrameIndex;
	UINT dxgiFactoryFlag = 0;

#ifdef _DEBUG
	CreateDebugInterface(&debug);

	debug.Get()->SetEnableAutoName(true);
	debug->EnableDebugLayer();
	dxgiFactoryFlag |= DXGI_CREATE_FACTORY_DEBUG;
#endif // DEBUG

	//Create factory interface to use it to get the adapters
	CreateFactory(dxgiFactoryFlag, &factory);

	//Go through available adapters and check that it's not a software adapter and that the adapter is valid by checking if it can create a device successfully
	GetDisplayAdapter(&adapter, factory.Get());

	//Get adapter info 
	DisplayAdapterInfo(adapter.Get());

	//Create device
	CreateDevice(adapter.Get(), &device);



	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.NodeMask = 0;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;

	if (FAILED(device->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), (LPVOID*)&queue)))
	{
		DEBUG("Failed creating queue");
		return -1;
	}

	queue.Get()->SetName(L"MainQueue");
	DEBUG("Command Queue Created");


	//create command list
	if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (LPVOID*)&cmdAllocator)))
	{
		DEBUG("Failed creating command allocator");
		return -1;
	}
	DEBUG("Command allocator created");


	//this creates a closed command list that doesnt have a pso or allocator bound to it
	if (FAILED(device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, __uuidof(ID3D12GraphicsCommandList7), (LPVOID*)&cmdList)))
	{
		DEBUG("Failed creating command list");
		return -1;
	}
	DEBUG("Command list created");



	//create swap chain
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.BufferCount = 2;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.Width = window.GetWidth();
	swapchainDesc.Height = window.GetHeight();
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	//swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	//swapchainDesc.Flags = ;
	//swapchainDesc.Stereo;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapchain1;
	if (FAILED(factory->CreateSwapChainForHwnd(queue.Get(), window.GetWindHandle(), &swapchainDesc, nullptr, nullptr, swapchain1.GetAddressOf())))
	{
		DEBUG("Failed creating swapchain");
		return -1;
	}

	swapchain1.As(&swapchain);
	DEBUG("Swapchain created");

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NodeMask = 0;
	rtvHeapDesc.NumDescriptors = 2;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;


	if (FAILED(device->CreateDescriptorHeap(&rtvHeapDesc, __uuidof(ID3D12DescriptorHeap), (LPVOID*)&rtvHeap)))
	{
		DEBUG("Failed creating swapchain rtv descriptor heap");
		return -1;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE	rtvHeapHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHeapOffset = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (int i = 0; i < 2; i++)
	{
		swapchain->GetBuffer(i, __uuidof(ID3D12Resource), (LPVOID*)&frameBuffers[i]);

		device->CreateRenderTargetView(frameBuffers[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.ptr += rtvHeapOffset;
		DEBUG("Backbuffer rtv created");
	}

	if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence1), (LPVOID*)&fence)))
	{
		DEBUG("Failed creating fence");
		return -1;
	}
	fenceValue = 1;
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	DEBUG("Created fence and fence event");
	


	//create depth/stencil buffer resource
	//create descriptor heap for depth stencil view

	D3D12_RESOURCE_DESC dsResourceDesc = {};
	dsResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	dsResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsResourceDesc.MipLevels = 1;
	dsResourceDesc.Width = window.GetWidth();
	dsResourceDesc.Height = window.GetHeight();
	dsResourceDesc.DepthOrArraySize = 1;
	dsResourceDesc.SampleDesc.Count = 1;
	dsResourceDesc.SampleDesc.Quality = 0;
	dsResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	dsResourceDesc.Alignment = 0;
	dsResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES dsHeapProperty = {};
	dsHeapProperty.Type = D3D12_HEAP_TYPE_DEFAULT;
	dsHeapProperty.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	dsHeapProperty.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	dsHeapProperty.CreationNodeMask = 0;
	dsHeapProperty.VisibleNodeMask = 0;


	float clearCol[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	D3D12_CLEAR_VALUE dsClearVal = {};
	dsClearVal.DepthStencil.Depth = 1;
	dsClearVal.DepthStencil.Stencil = 0;
	dsClearVal.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;


	if (FAILED(device->CreateCommittedResource(&dsHeapProperty, D3D12_HEAP_FLAG_NONE, &dsResourceDesc, D3D12_RESOURCE_STATE_COMMON, &dsClearVal, __uuidof(ID3D12Resource), (LPVOID*)&depthBuffer)))
	{
		DEBUG("Failed creating depth stencil buffer resource");
		return -1;
	}

	DEBUG("depth stencil resource created");

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NodeMask = 0;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.Texture2D.MipSlice = 0;


	if (FAILED(device->CreateDescriptorHeap(&dsvHeapDesc, __uuidof(ID3D12DescriptorHeap), (LPVOID*)&dsvHeap)))
	{
		DEBUG("Failed creating depth stencil view descriptor heap");
		return -1;
	}

	DEBUG("depth stencil view descriptor heap created");
	
	device->CreateDepthStencilView(depthBuffer.Get(), &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());
	DEBUG("depth stencil view created");

	//binds allocator and pso to command list and opens the list for recording
	cmdList->Reset(cmdAllocator.Get(), pipelineState.Get());

	//transition depth resource state from common to dsv
	//need to have command list opened first to submit this command
	D3D12_RESOURCE_BARRIER dsBarrier = {};
	dsBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	dsBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	dsBarrier.Transition.pResource = depthBuffer.Get();
	dsBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	dsBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	cmdList->ResourceBarrier(1, &dsBarrier);

	//Create and set viewport
	D3D12_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(window.GetWidth());
	viewport.Height = static_cast<float>(window.GetHeight());
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	cmdList->RSSetViewports(1, &viewport);
	
	currentFrameIndex = swapchain->GetCurrentBackBufferIndex();
	cmdList->Close();

	ID3D12CommandList* ppCommandLists[] = { cmdList.Get() };
	queue->ExecuteCommandLists(1, ppCommandLists);


	{
		const UINT64 fenceVal = fenceValue;
		queue->Signal(fence.Get(), fenceVal);
		fenceValue++;

		if (fence->GetCompletedValue() < fenceVal)
		{
			fence->SetEventOnCompletion(fenceVal, fenceEvent);
			WaitForSingleObject(fenceEvent, INFINITE);
		}
	}


	//main loop
	while (true)
	{
		if (window.ProcessMessages() == -1)
		{
			break;
		}

		//Render code
		{
			//Reset cmd allocator
			cmdAllocator->Reset();
			//Reset cmd 
			cmdList->Reset(cmdAllocator.Get(), pipelineState.Get());
			//transition current back buffer from present state to render target state
			D3D12_RESOURCE_BARRIER frameBufferBarrier = {};
			frameBufferBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			frameBufferBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			frameBufferBarrier.Transition.pResource = frameBuffers[currentFrameIndex].Get();
			frameBufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			frameBufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			cmdList->ResourceBarrier(1, &frameBufferBarrier);
			//clear render target to color
			D3D12_CPU_DESCRIPTOR_HANDLE	rtvHeapHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
			rtvHeapOffset = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			rtvHeapHandle.ptr += rtvHeapOffset * currentFrameIndex;

			cmdList->ClearRenderTargetView(rtvHeapHandle, clearCol, 0, nullptr);
			//transition current back buffer from render target state to present state
			frameBufferBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			frameBufferBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			frameBufferBarrier.Transition.pResource = frameBuffers[currentFrameIndex].Get();
			frameBufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			frameBufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			cmdList->ResourceBarrier(1, &frameBufferBarrier);
			//close command list
			cmdList->Close();

			//execute command list from command queue
			ppCommandLists[0] = { cmdList.Get() };
			queue->ExecuteCommandLists(1, ppCommandLists);
			//swapchain present
			swapchain->Present(1, 0);
			//wait for fence
			{
				const UINT64 fenceVal = fenceValue;
				queue->Signal(fence.Get(), fenceVal);
				fenceValue++;

				if (fence->GetCompletedValue() < fenceVal)
				{
					fence->SetEventOnCompletion(fenceVal, fenceEvent);
					WaitForSingleObject(fenceEvent, INFINITE);
				}
				currentFrameIndex = swapchain->GetCurrentBackBufferIndex();
			}
		}
	}

	return 0;
}