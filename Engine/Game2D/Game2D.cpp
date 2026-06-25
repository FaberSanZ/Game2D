// Game2D.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

int main()
{
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* cmd = nullptr;
	IDXGISwapChain* swapChain = nullptr;
	ID3D11RenderTargetView* rtv = nullptr;
	ID3D11Texture2D* backBuffer = nullptr;



	uint32_t width = 1220;
	uint32_t height = 820;

	const wchar_t* title = L"Game2D";	

	WNDCLASSEX wcex = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, title, nullptr };
	RegisterClassEx(&wcex);


	HWND hwnd = CreateWindow(title, title, WS_OVERLAPPEDWINDOW, 100, 100, width, height, nullptr, nullptr, wcex.hInstance, nullptr);
	ShowWindow(hwnd, SW_SHOW);


	// Create device and swap chain
	uint32_t frameBufferCount = 2;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = frameBufferCount;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = true;

	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, nullptr, &cmd);

	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	device->CreateRenderTargetView(backBuffer, nullptr, &rtv);
	backBuffer->Release();

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		// Clear the back buffer
		float clearColor[4] = { 0.2f, 0.3f, 0.3f, 1.0f };
		cmd->ClearRenderTargetView(rtv, clearColor);

		swapChain->Present(1, 0);
	}


	// Cleanup
	swapChain->Release();
	cmd->Release();
	device->Release();


}


