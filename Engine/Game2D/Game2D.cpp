#include <iostream>
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include <vector>
#include <wincodec.h>
#include <DirectXMath.h>
#include <vector>


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "windowscodecs.lib")

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


	ID3D11Buffer* buffer = nullptr;

	if (data)
	{
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = data;

		device->CreateBuffer(&bufferDesc, &initData, &buffer);
	}
	else
	{
		device->CreateBuffer(&bufferDesc, nullptr, &buffer);
	}


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


ID3D11Buffer* CreateIndexBuffer(ID3D11Device* device, const void* data, uint32_t size)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = size;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = data;

	ID3D11Buffer* buffer = nullptr;
	device->CreateBuffer(&bufferDesc, &initData, &buffer);

	return buffer;
}


ID3D11ShaderResourceView* LoadTextureWIC(ID3D11Device* device, const wchar_t* filePath)
{
	IWICImagingFactory* factory = nullptr;
	IWICBitmapDecoder* decoder = nullptr;
	IWICBitmapFrameDecode* frame = nullptr;
	IWICFormatConverter* converter = nullptr;

	CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));

	factory->CreateDecoderFromFilename(filePath, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
	decoder->GetFrame(0, &frame);

	UINT width = 0;
	UINT height = 0;
	frame->GetSize(&width, &height);

	factory->CreateFormatConverter(&converter);
	converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);

	UINT stride = width * 4;
	UINT imageSize = stride * height;

	std::vector<uint8_t> pixels(imageSize);
	converter->CopyPixels(nullptr, stride, imageSize, pixels.data());

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = pixels.data();
	initData.SysMemPitch = stride;

	ID3D11Texture2D* texture = nullptr;
	device->CreateTexture2D(&textureDesc, &initData, &texture);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	ID3D11ShaderResourceView* textureView = nullptr;
	device->CreateShaderResourceView(texture, &srvDesc, &textureView);

	if (texture) texture->Release();
	if (converter) converter->Release();
	if (frame) frame->Release();
	if (decoder) decoder->Release();
	if (factory) factory->Release();

	return textureView;
}


ID3D11SamplerState* CreateTextureSampler(ID3D11Device* device)
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* sampler = nullptr;
	device->CreateSamplerState(&samplerDesc, &sampler);

	return sampler;
}

ID3D11BlendState* CreateAlphaBlendState(ID3D11Device* device)
{
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;

	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	ID3D11BlendState* blendState = nullptr;
	device->CreateBlendState(&blendDesc, &blendState);

	return blendState;
}

ID3D11Buffer* CreateConstantBuffer(ID3D11Device* device, uint32_t size)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = size;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	ID3D11Buffer* buffer = nullptr;
	device->CreateBuffer(&bufferDesc, nullptr, &buffer);
	return buffer;
}


int main()
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

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
	ID3D11Buffer* indexBuffer = nullptr;

	ID3D11ShaderResourceView* textureView = nullptr;
	ID3D11SamplerState* textureSampler = nullptr;
	ID3D11BlendState* alphaBlendState = nullptr;

	ID3D11Buffer* cameraBuffer = nullptr;


	ID3D11Buffer* instacingBuffer = nullptr;
	ID3D11ShaderResourceView* instacingView = nullptr;

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

	// Define vertices for a triangle
	struct Vertex
	{
		float position[4];
		float color[4];
		float uv[2];
	};


	Vertex vertices[] =
	{
		{ -0.5,  0.5, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f, 1.0f,   0.0f, 0.0f },
		{  0.5,  0.5, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 0.0f },
		{  0.5, -0.5, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 1.0f },
		{ -0.5, -0.5, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f, 1.0f,   0.0f, 1.0f }
	};
	vertexBuffer = CreateStructuredBuffer(device, vertices, sizeof(Vertex), 4);
	vertexView = CreateStructuredBufferView(device, vertexBuffer, 4);


	uint32_t indices[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	indexBuffer = CreateIndexBuffer(device, indices, sizeof(indices));

	textureView = LoadTextureWIC(device, L"Assets/deco_check_bw_01_ffcc00_512.png");
	textureSampler = CreateTextureSampler(device);



	alphaBlendState = CreateAlphaBlendState(device);



	const float halfVisibleHeight = 2.0f;
	const float aspectRatio = (float)width / (float)height;
	const float halfVisibleWidth = halfVisibleHeight * aspectRatio;

	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixOrthographicOffCenterLH(-halfVisibleWidth, halfVisibleWidth, -halfVisibleHeight, halfVisibleHeight, 0.0f, 1.0f);

	cameraBuffer = CreateConstantBuffer(device, sizeof(DirectX::XMMATRIX));
	std::vector<DirectX::XMMATRIX> instanceMatrices;
	instanceMatrices.reserve(8 * 8);

	//DirectX::XMMATRIX meshMatrix =
	//	DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) *
	//	DirectX::XMMatrixRotationZ(0.1f) *
	//	DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	//instanceMatrices.push_back(DirectX::XMMatrixTranspose(meshMatrix));

	//meshMatrix =
	//	DirectX::XMMatrixScaling(0.5f, 0.5f, 1.0f) *
	//	DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.4f) *
	//	DirectX::XMMatrixTranslation(1.3f, 1.3f, 0.0f);

	//instanceMatrices.push_back(DirectX::XMMatrixTranspose(meshMatrix));

	const uint32_t instanceCount = 8 * 8;

	instacingBuffer = CreateStructuredBuffer(device, nullptr, sizeof(DirectX::XMMATRIX), instanceCount);
	instacingView = CreateStructuredBufferView(device, instacingBuffer, instanceCount);



	float gameTime = 0.0f;

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		instanceMatrices.clear();

		for(float x = -2.0f; x < 2.0f; x += 0.5f)
		{
			for(float y = -2.0f; y < 2.0f; y += 0.5f)
			{
				DirectX::XMMATRIX meshMatrix = DirectX::XMMatrixScaling(0.2f, 0.2f, 1.0f) * DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, gameTime) * DirectX::XMMatrixTranslation(x, y, 0.0f);
				instanceMatrices.push_back(DirectX::XMMatrixTranspose(meshMatrix));
			}
		}

		cmd->UpdateSubresource(instacingBuffer, 0, nullptr, instanceMatrices.data(), 0, 0);

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

		float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		cmd->OMSetBlendState(alphaBlendState, blendFactor, 0xffffffff);

		cmd->UpdateSubresource(cameraBuffer, 0, nullptr, &projectionMatrix, 0, 0);
		
		// Draw a mesh
		cmd->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmd->VSSetShader(vertexShader, nullptr, 0);
		cmd->PSSetShader(pixelShader, nullptr, 0);
		//cmd->IASetInputLayout(nullptr); // No input layout needed for structured buffer
		cmd->VSSetShaderResources(0, 1, &vertexView);
		cmd->PSSetShaderResources(0, 1, &textureView);
		cmd->PSSetSamplers(0, 1, &textureSampler);
		cmd->VSSetConstantBuffers(0, 1, &cameraBuffer);
		cmd->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		cmd->VSSetShaderResources(1, 1, &instacingView);
		cmd->DrawIndexedInstanced(6, 560, 0, 0, 0);

		swapChain->Present(1, 0);

		gameTime += 0.016f;
	}

	// Cleanup
	if (indexBuffer) indexBuffer->Release();
	if (vertexShader) vertexShader->Release();
	if (pixelShader) pixelShader->Release();
	if (vsBlob) vsBlob->Release();
	if (psBlob) psBlob->Release();
	if (rtv) rtv->Release();
	if (swapChain) swapChain->Release();
	if (cmd) cmd->Release();
	if (device) device->Release();

	CoUninitialize();

}