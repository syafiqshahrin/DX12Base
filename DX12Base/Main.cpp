#include "Windows.h"
#include "Window.h"
#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>
#include "Debug.h"

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
		}
		
		

	}




	while (true)
	{
		if (window.ProcessMessages() == -1)
		{
			break;
		}
	}
	
	
	//factory.Get()->Release();
	//queue.Get()->Release();
	device.Get()->Release();
	adapter.Get()->Release();
	debug.Get()->Release();


	return 0;
}