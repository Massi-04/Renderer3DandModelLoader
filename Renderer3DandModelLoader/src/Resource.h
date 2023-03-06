#pragma once

#include "Core.h"

#include "Mesh.h"
#include "Texture.h"

template<typename T>
static bool GetFalse()
{
	return false;
}

Mesh* LoadMeshFromFile(const String& path);
Texture* LoadTextureFromFile(const String& path);

class Resource
{
public:
	Resource() = delete;

	template<typename T>
	static T* Load(const String& path)
	{
		static_assert(GetFalse<T>(), "Not implemented!");
		return nullptr;
	}

	template<>
	static Mesh* Load<Mesh>(const String& path)
	{
		return LoadMeshFromFile(path);
	}

	template<>
	static Texture* Load<Texture>(const String& path)
	{
		return LoadTextureFromFile(path);
	}
};