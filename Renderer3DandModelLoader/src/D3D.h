#pragma once

#include "Core.h"
#include <d3d11.h>

extern IDXGISwapChain* GSwapChain;
extern ID3D11Device* GDevice;
extern ID3D11DeviceContext* GContext;
extern ID3D11RenderTargetView* GRenderTargetView;

enum EBindFlags
{
	VertexBuffer = D3D11_BIND_INDEX_BUFFER,
	IndexBuffer = D3D11_BIND_VERTEX_BUFFER,
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
	Default = 0,
	Immutable = 1,
	Dynamic = 2,
	Staging = 3
};

enum EShaderType
{
	VertexShader,
	PixelShader
};

void InitD3D();

ID3D11Buffer* CreateBuffer(EBindFlags type, ECPUAccessFlags cpuAccessFlags, size_t sizeBytes, void* data = nullptr);
ID3D11Texture2D* CreateTexture2D(uint32_t width, uint32_t height, EColorFormat format, EResourseUsage usage, EBindFlags bindType, ECPUAccessFlags cpuAccessFlags, void* data = nullptr);
ID3DBlob* CompileShader(const WString& file, EShaderType type);
ID3D11VertexShader* CreateVertexShader(ID3DBlob* compiledShader);
ID3D11PixelShader* CreatePixelShader(ID3DBlob* compiledShader);