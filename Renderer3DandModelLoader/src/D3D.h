#pragma once

#include "Core.h"
#include <d3d11.h>

extern bool GD3DInitalized;
extern IDXGISwapChain* GSwapChain;
extern ID3D11Device* GDevice;
extern ID3D11DeviceContext* GContext;
extern ID3D11RenderTargetView* GRenderTargetView;
extern ID3D11DepthStencilView* GDepthBufferView;
extern ID3D11RasterizerState* GRasterizerState;
extern ID3D11SamplerState* GSamplerState;

enum EBindFlags
{
	VertexBuffer = D3D11_BIND_VERTEX_BUFFER,
	IndexBuffer = D3D11_BIND_INDEX_BUFFER,
	ConstBuffer = D3D11_BIND_CONSTANT_BUFFER,
	RenderTarget = D3D11_BIND_RENDER_TARGET,
	DepthBuffer = D3D11_BIND_DEPTH_STENCIL,
	ShaderResource = D3D11_BIND_SHADER_RESOURCE
};

enum ECPUAccessFlags
{
	None = 0,
	Write = D3D11_CPU_ACCESS_WRITE,
	Read = D3D11_CPU_ACCESS_READ
};

enum EColorFormat
{
	RGBA = DXGI_FORMAT_R8G8B8A8_UNORM,
	DepthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT
};

enum EResourseUsage
{
	Default = D3D11_USAGE_DEFAULT,
	Immutable = D3D11_USAGE_IMMUTABLE,
	Dynamic = D3D11_USAGE_DYNAMIC,
	Staging = D3D11_USAGE_STAGING
};

enum EShaderType
{
	VertexShader,
	PixelShader
};

enum EFillMode
{
	Wireframe = D3D11_FILL_WIREFRAME,
	FillSolid = D3D11_FILL_SOLID
};

enum ECullMode
{
	NoCull = D3D11_CULL_NONE,
	Front = D3D11_CULL_FRONT,
	Back = D3D11_CULL_BACK
};

struct RasterizerDesc
{
	EFillMode FillMode;
	ECullMode CullMode;
};

void InitD3D();

ID3D11Buffer* CreateBuffer(EResourseUsage usage, EBindFlags type, ECPUAccessFlags cpuAccessFlags, size_t sizeBytes, void* data = nullptr);
ID3D11Texture2D* CreateTexture2D(uint32_t width, uint32_t height, EColorFormat format, EResourseUsage usage, EBindFlags bindType, ECPUAccessFlags cpuAccessFlags, void* data = nullptr, uint32_t dataPitch = 0);
ID3DBlob* CompileShader(const WString& file, EShaderType type);
ID3D11VertexShader* CreateVertexShader(ID3DBlob* compiledShader);
ID3D11PixelShader* CreatePixelShader(ID3DBlob* compiledShader);

void ResizeFrameBuffer(uint32_t newClientWidth, uint32_t newClientHeight);
RasterizerDesc GetCurrentRasterizerDesc();
void SetRasterizerState(RasterizerDesc desc);
void SetFillMode(EFillMode fillMode);
void SetCullMode(ECullMode cullMode);
void SetFullscreen(bool fullscreen);

