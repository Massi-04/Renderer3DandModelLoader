#include "TextureLoader.h"

#include "vendor/stb/stb_image.h"

Texture* LoadTexture(const char* filePath)
{
	int width, height, channels;
	unsigned char* data = stbi_load(filePath, &width, &height, &channels, DEFAULT_TEX_CHANNELS);

	TextureData tData = { data, String(filePath), width, height, channels };

	Texture* res = new Texture(tData);

	stbi_image_free(data);

	return res;
}
