#include "Windows.h"
#include "Window.h"
#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>
#include "Debug.h"
#include <vector>

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
	


	UINT rtvHeapOffset;
	
	
	UINT currentFrameIndex;

	UINT dxgiFactoryFlag = 0;

	if (SUCCEEDED(D3D12GetDebugInterface(__uuidof(ID3D12Debug6), (LPVOID*)&debug)))
	{
		debug.Get()->SetEnableAutoName(true);
		debug->EnableDebugLayer();
		dxgiFactoryFlag |= DXGI_CREATE_FACTORY_DEBUG;

	}

	if (SUCCEEDED(CreateDXGIFactory2(dxgiFactoryFlag, __uuidof(IDXGIFactory6), (LPVOID*)&factory)))
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter1;
		for (UINT adapterIndex = 0; SUCCEEDED(factory->EnumAdapters1(adapterIndex, &adapter1)); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter1->GetDesc1(&desc);
			
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}

			if (SUCCEEDED(D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
			{
				break;
			}

		}

		adapter1->QueryInterface(_uuidof(IDXGIAdapter4), (LPVOID*)&adapter);
		adapter1.Detach();
		adapter1.Reset();
	}

	DXGI_ADAPTER_DESC3 desc;
	adapter->GetDesc3(&desc);
	DEBUG(desc.Description);
	DEBUG(desc.DedicatedVideoMemory);

	if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device10), (LPVOID*)&device)))
	{
		device.Get()->SetName(L"MainDevice");
		DEBUG("Device Created");

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.NodeMask = 0;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;

		if (SUCCEEDED(device->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), (LPVOID*)&queue)))
		{
			queue.Get()->SetName(L"MainQueue");
			DEBUG("Command Queue Created");

			DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
			swapchainDesc.BufferCount = 2;
			swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapchainDesc.Width = window.GetWidth();
			swapchainDesc.Height= window.GetHeight();
			swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapchainDesc.SampleDesc.Count = 1;
			swapchainDesc.SampleDesc.Quality = 0;
			//swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
			//swapchainDesc.Flags = ;
			//swapchainDesc.Stereo;
			
			Microsoft::WRL::ComPtr<IDXGISwapChain1> swapchain1;
			if (SUCCEEDED(factory->CreateSwapChainForHwnd(queue.Get(), window.GetWindHandle(), &swapchainDesc, nullptr, nullptr, swapchain1.GetAddressOf())))
			{
				//swapchain1->QueryInterface(__uuidof(IDXGISwapChain4), (LPVOID*)&swapchain);
				swapchain1.As(&swapchain);
				swapchain1.Detach();
				DEBUG("Swapchain created");

				D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
				rtvHeapDesc.NodeMask = 0;
				rtvHeapDesc.NumDescriptors = 2;
				rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
				rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;


				if (SUCCEEDED(device->CreateDescriptorHeap(&rtvHeapDesc, __uuidof(ID3D12DescriptorHeap), (LPVOID*)&rtvHeap)))
				{
					D3D12_CPU_DESCRIPTOR_HANDLE	rtvHeapHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
					rtvHeapOffset = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

					for (int i = 0; i < 2; i++)
					{
						swapchain->GetBuffer(i, __uuidof(ID3D12Resource), (LPVOID*)&frameBuffers[i]);

						device->CreateRenderTargetView(frameBuffers[i].Get(), nullptr, rtvHeapHandle);
						rtvHeapHandle.ptr += rtvHeapOffset;
						DEBUG("Backbuffer rtv created");
					}

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


					float clearCol[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
					D3D12_CLEAR_VALUE dsClearVal = {};
					dsClearVal.DepthStencil.Depth = 1;
					dsClearVal.DepthStencil.Stencil = 0;
					dsClearVal.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
					


					if (SUCCEEDED(device->CreateCommittedResource(&dsHeapProperty, D3D12_HEAP_FLAG_NONE, &dsResourceDesc, D3D12_RESOURCE_STATE_COMMON, &dsClearVal, __uuidof(ID3D12Resource), (LPVOID*)&depthBuffer)))
					{
						DEBUG("depth stencil resource created");

						D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
						dsvHeapDesc.NodeMask = 0;
						dsvHeapDesc.NumDescriptors = 2;
						dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
						dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

						D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
						dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
						dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
						dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
						dsvDesc.Texture2D.MipSlice = 0;


						if (SUCCEEDED(device->CreateDescriptorHeap(&dsvHeapDesc, __uuidof(ID3D12DescriptorHeap), (LPVOID*)&dsvHeap)))
						{
							device->CreateDepthStencilView(depthBuffer.Get(), &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());
							DEBUG("depth stencil view created");
						}


					}
				}

			}

			



		}

	}




	while (true)
	{
		if (window.ProcessMessages() == -1)
		{
			break;
		}
	}

	rtvHeap.Get()->Release();
	swapchain.Get()->Release();
	//factory.Get()->Release();
	//queue.Get()->Release();
	device.Get()->Release();
	adapter.Get()->Release();
	debug.Get()->Release();


	return 0;
}