#pragma once

#include "Core.h"

#define DEFAULT_TEX_CHANNELS 4

struct TextureData
{
	uint8_t* Data;
	String Name;
	uint32_t Width;
	uint32_t Height;
	uint16_t NumChannels;
};

class Texture
{
public:
	Texture(TextureData texData);
	~Texture();

	void Bind(uint32_t slot = 0);
	void Unbind();

	inline struct ID3D11Texture2D* GetTextureHandle() const { return m_Texture; }
	inline struct ID3D11ShaderResourceView* GetTextureViewHandle() const { return m_TexView; }
	inline const String& GetName() const { return m_Name; }
	inline const uint32_t GetWidth() const { return m_Width; }
	inline const uint32_t GetHeight() const { return m_Height; }
	inline uint16_t GetNumChannels() const { return m_NumChannels; }

private:
	struct ID3D11Texture2D* m_Texture;
	struct ID3D11ShaderResourceView* m_TexView;
	String m_Name;
	uint32_t m_Width;
	uint32_t m_Height;
	uint16_t m_NumChannels;
};

