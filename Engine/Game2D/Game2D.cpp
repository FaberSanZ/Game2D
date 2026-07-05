#include <iostream>
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

void CompileShaderFromFile(const wchar_t* filePath, const char* entryPoint, const char* shaderModel, ID3DBlob** blob)
{
	D3DCompileFromFile(filePath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel, 0, 0, blob, nullptr);
}

ID3D11Buffer* CreateStructuredBuffer(ID3D11Device* device, const void* data, uint32_t stride, uint32_t count)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = stride * count;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = stride;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = data;

	ID3D11Buffer* buffer = nullptr;
	device->CreateBuffer(&bufferDesc, &initData, &buffer);

	return buffer;
}

ID3D11ShaderResourceView* CreateStructuredBufferView(ID3D11Device* device, ID3D11Buffer* buffer, uint32_t count)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = count;

	ID3D11ShaderResourceView* view = nullptr;
	device->CreateShaderResourceView(buffer, &srvDesc, &view);

	return view;
}

int main()
{
	uint32_t width = 1220;
	uint32_t height = 820;
	const wchar_t* title = L"Game2D";	

	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* cmd = nullptr;
	IDXGISwapChain* swapChain = nullptr;
	ID3D11RenderTargetView* rtv = nullptr;
	ID3D11Texture2D* backBuffer = nullptr;
	uint32_t frameBufferCount = 2;

	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;

	// mesh
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11ShaderResourceView* vertexView = nullptr;


	WNDCLASSEX wcex = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, title, nullptr };
	RegisterClassEx(&wcex);

	HWND hwnd = CreateWindow(title, title, WS_OVERLAPPEDWINDOW, 100, 100, width, height, nullptr, nullptr, wcex.hInstance, nullptr);
	ShowWindow(hwnd, SW_SHOW);

	// Create device and swap chain
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

	ID3DBlob* vsBlob = nullptr;
	CompileShaderFromFile(L"Assets/Vertex.hlsl", "VS", "vs_5_0", &vsBlob);

	ID3DBlob* psBlob = nullptr;
	CompileShaderFromFile(L"Assets/Pixel.hlsl", "PS", "ps_5_0", &psBlob);

	device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
	device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);


	float vertices[] = 
	{
		0.0f,  0.5f, 0.0f,  // Top vertex
		0.5f, -0.5f, 0.0f,  // Bottom right vertex
	   -0.5f, -0.5f, 0.0f   // Bottom left vertex
	};

	vertexBuffer = CreateStructuredBuffer(device, vertices, sizeof(float) * 3, 3);
	vertexView = CreateStructuredBufferView(device, vertexBuffer, 3);

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		cmd->OMSetRenderTargets(1, &rtv, nullptr);

		// Clear the back buffer
		float clearColor[4] = { 0.2f, 0.3f, 0.3f, 1.0f };
		cmd->ClearRenderTargetView(rtv, clearColor);

		// Set the viewport
		D3D11_VIEWPORT viewport = {};
		viewport.Width = (float)width;
		viewport.Height = (float)height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		cmd->RSSetViewports(1, &viewport);

		// Draw a triangle
		cmd->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmd->VSSetShader(vertexShader, nullptr, 0);
		cmd->PSSetShader(pixelShader, nullptr, 0);
		cmd->IASetInputLayout(nullptr);
		cmd->VSSetShaderResources(0, 1, &vertexView);

		cmd->Draw(3, 0);

		swapChain->Present(1, 0);
	}

	// Cleanup
	if (vertexShader) vertexShader->Release();
	if (pixelShader) pixelShader->Release();
	if (vsBlob) vsBlob->Release();
	if (psBlob) psBlob->Release();
	if (rtv) rtv->Release();
	if (swapChain) swapChain->Release();
	if (cmd) cmd->Release();
	if (device) device->Release();
}