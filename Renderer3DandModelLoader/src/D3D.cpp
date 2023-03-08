#include "D3D.h"
#include "Window.h"
#include <d3dcompiler.h>

bool GD3DInitalized = false;
IDXGISwapChain* GSwapChain = nullptr;
ID3D11Device* GDevice = nullptr;
ID3D11DeviceContext* GContext = nullptr;
ID3D11RenderTargetView* GRenderTargetView = nullptr;
ID3D11DepthStencilView* GDepthBufferView = nullptr;
ID3D11RasterizerState* GRasterizerState = nullptr;
ID3D11SamplerState* GSamplerState = nullptr;

static const char const* SHADER_ENTRY_POINT = "main";

static const char* shaderTargets[] =
{
	"vs_5_0",	// EShaderType::VertexShader
	"ps_5_0"	// EShaderType::PixelShader
};

static ID3D11Texture2D* s_DepthBuffer;

static RasterizerDesc sRasterDesc = { FillSolid, Back } ;

void InitD3D()
{
    const WindowProps& wndProps = GetWndProps();

	// setup swapchain and create device

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = wndProps.Width;
    sd.BufferDesc.Height = wndProps.Height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.BufferDesc.RefreshRate.Numerator = wndProps.Refreshrate;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = GWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = !wndProps.Fullscreen;

    checkf
    (
        D3D11CreateDeviceAndSwapChain
        (
            nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, D3D11_CREATE_DEVICE_DEBUG, 0, 0, D3D11_SDK_VERSION, &sd, &GSwapChain, &GDevice, 0, &GContext
        ) == S_OK,
        "Impossibile creare device e swapchain"
    );

	// setup rendertarget and depthbuffer

	ResizeFrameBuffer(wndProps.Width, wndProps.Height);

	SetRasterizerState(sRasterDesc);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	checkf(GDevice->CreateSamplerState(&samplerDesc, &GSamplerState) == S_OK, 
		"impossibile creare il sampler state!");

	GContext->PSSetSamplers(0, 1, &GSamplerState);

	GD3DInitalized = true;
}

ID3D11Buffer* CreateBuffer(EResourseUsage usage, EBindFlags type, ECPUAccessFlags cpuAccessFlags, size_t sizeBytes, void* data)
{
	ID3D11Buffer* res = nullptr;

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = (D3D11_USAGE)usage;
	bufferDesc.ByteWidth = (UINT)sizeBytes;
	bufferDesc.BindFlags = type;
	bufferDesc.CPUAccessFlags = cpuAccessFlags;
	bufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = data;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	checkf(GDevice->CreateBuffer(&bufferDesc, data ? &initData : 0, &res) == S_OK, "impossibile creare il buffer!");

	return res;
}

ID3D11Texture2D* CreateTexture2D(uint32_t width, uint32_t height, EColorFormat format, EResourseUsage usage, EBindFlags bindType, ECPUAccessFlags cpuAccessFlags, void* data, uint32_t dataPitch)
{
	ID3D11Texture2D* res = nullptr;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = (DXGI_FORMAT)format;
	desc.SampleDesc.Count = 1;
	desc.Usage = (D3D11_USAGE)usage;
	desc.BindFlags = bindType;
	desc.CPUAccessFlags = cpuAccessFlags;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA texData = {};
	texData.pSysMem = data;
	texData.SysMemPitch = (UINT)dataPitch;
	texData.SysMemSlicePitch = 0;

	checkf(GDevice->CreateTexture2D(&desc, data ? &texData : 0, &res) == S_OK, "impossibile creare la texture2d");

	return res;
}

ID3DBlob* CompileShader(const WString& file, EShaderType type)
{	
	ID3DBlob* compileResult;
	ID3DBlob* compileErrors;

	checkf(D3DCompileFromFile
	(
		file.c_str(), nullptr, /* macro  */ nullptr, /* include */
		SHADER_ENTRY_POINT, shaderTargets[type], 0, 0, &compileResult,
		&compileErrors
	) == S_OK, "impossibile compilare lo shader!");

	checkf(!compileErrors, (const char*)compileErrors->GetBufferPointer());

	return compileResult;
}

ID3D11VertexShader* CreateVertexShader(ID3DBlob* compiledShader)
{
	ID3D11VertexShader* res;

	checkf(GDevice->CreateVertexShader
	(
		compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), nullptr, &res
	) == S_OK, "impossibile creare la vertex shader");

	return res;
}

ID3D11PixelShader* CreatePixelShader(ID3DBlob* compiledShader)
{
	ID3D11PixelShader* res;

	checkf(GDevice->CreatePixelShader
	(
		compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), nullptr, &res
	) == S_OK, "impossibile creare la pixel shader");

	return res;
}

void ResizeFrameBuffer(uint32_t newClientWidth, uint32_t newClientHeight)
{
	if (!newClientWidth && !newClientHeight)
		return;

	GContext->OMSetRenderTargets(0, 0, 0);

	if(GRenderTargetView)
		GRenderTargetView->Release();

	checkf(GSwapChain->ResizeBuffers(0, newClientWidth, newClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0) == S_OK,
		"impossibile resizare il back buffer!");

	ID3D11Texture2D* backBuffer;
	
	checkf(GSwapChain->GetBuffer(0 /*first buffer (back buffer)*/, __uuidof(ID3D11Texture2D), (void**)&backBuffer)
		== S_OK, "impossibile trovare il backbuffer ???");

	checkf(GDevice->CreateRenderTargetView(backBuffer, nullptr, &GRenderTargetView) == S_OK,
		"impossibile creare il nuovo rendertargetview");

	backBuffer->Release();

	if(GDepthBufferView)
		GDepthBufferView->Release();

	if(s_DepthBuffer)
		s_DepthBuffer->Release();

	s_DepthBuffer = CreateTexture2D(newClientWidth, newClientHeight, DepthBufferFormat, Default, DepthBuffer, None);
	checkf(GDevice->CreateDepthStencilView(s_DepthBuffer, nullptr, &GDepthBufferView) == S_OK, "impossible creare il depth buffer view");


	GContext->OMSetRenderTargets(1, &GRenderTargetView, GDepthBufferView);

	// Set up the viewport.
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = newClientWidth;
	viewport.Height = newClientHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	GContext->RSSetViewports(1, &viewport);

	DXGI_SWAP_CHAIN_DESC updatedDesc;

	checkf(GSwapChain->GetDesc(&updatedDesc) == S_OK, "impossibile gettare la swapchiaindesc ?!?!");

	WindowProps& propsRef = GetWndPropsUnsafe();

	propsRef.Refreshrate = updatedDesc.BufferDesc.RefreshRate.Numerator;
	propsRef.Fullscreen = !updatedDesc.Windowed;
}

void SetRasterizerState(RasterizerDesc desc)
{
	GContext->RSSetState(nullptr);

	if (GRasterizerState)
		GRasterizerState->Release();

	D3D11_RASTERIZER_DESC rasterizerDesc = {};

	rasterizerDesc.FillMode = (D3D11_FILL_MODE)desc.FillMode;
	rasterizerDesc.CullMode = (D3D11_CULL_MODE)desc.CullMode;

	checkf(GDevice->CreateRasterizerState(&rasterizerDesc, &GRasterizerState) == S_OK,
		"impossibile craere il rasterizer state");

	GContext->RSSetState(GRasterizerState);

	sRasterDesc = desc;
}

RasterizerDesc GetCurrentRasterizerDesc()
{
	return sRasterDesc;
}

void SetFillMode(EFillMode fillMode)
{
	SetRasterizerState( { fillMode, sRasterDesc.CullMode } );
}

void SetCullMode(ECullMode cullMode)
{
	SetRasterizerState({ sRasterDesc.FillMode, cullMode });
}

const char* GetFillModeStr(EFillMode fillMode)
{
	switch (fillMode)
	{
	case Wireframe: return "Wireframe";
	case FillSolid: return "Solid";
	default:        return "Unknown ?!?";
	}
}

const char* GetCullModeStr(ECullMode cullMode)
{
	switch (cullMode)
	{
	case NoCull: return "None";
	case Front:  return "Front";
	case Back:   return "Back";
	default:     return "Unknown ?!?";
	}
}

void SetFullscreen(bool fullscreen)
{
	checkf(GSwapChain->SetFullscreenState(fullscreen, nullptr) == S_OK, "impossibile toggleare il fullscreen :(");
}