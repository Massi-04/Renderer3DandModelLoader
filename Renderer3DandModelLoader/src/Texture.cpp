#include "Texture.h"
#include "D3D.h"

Texture::Texture(TextureData texData)
	: m_Texture(CreateTexture2D(texData.Width, texData.Height, RGBA, Default, ShaderResource, None, texData.Data, texData.Width * DEFAULT_TEX_CHANNELS))
	, m_Name(texData.Name), m_Width(texData.Width), m_Height(texData.Height), m_NumChannels(texData.NumChannels)
{
	check(GDevice->CreateShaderResourceView(m_Texture, nullptr, &m_TexView) == S_OK);
}

Texture::~Texture()
{
	m_TexView->Release();
	m_Texture->Release();
}

void Texture::Bind(uint32_t slot)
{
	GContext->PSSetShaderResources(0, 1, &m_TexView);
}

void Texture::Unbind()
{
	GContext->PSSetShaderResources(0, 0, nullptr);
}
