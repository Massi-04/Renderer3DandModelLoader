#include "Mesh.h"
#include "D3D.h"

Submesh::Submesh(const MeshData& meshData)
	: Texture(nullptr)
{	
	mProps.Name = meshData.Name;
	mProps.PolyCount = meshData.PolyCount;
	mProps.IndexCount = meshData.IndexBufferData.size();
	mProps.VertexCount = meshData.VertexBufferData.size();
	
	mVertexBuffer = CreateBuffer(Default, VertexBuffer, None, meshData.VertexBufferData.size() * sizeof(Vertex), (void*)meshData.VertexBufferData.data());
	mIndexBuffer = CreateBuffer(Default, IndexBuffer, None, meshData.IndexBufferData.size() * sizeof(uint32_t), (void*)meshData.IndexBufferData.data());
}

Submesh::~Submesh()
{
	mVertexBuffer->Release();
	mIndexBuffer->Release();
}

Mesh::Mesh(const String& name, const std::vector<MeshData>& meshData)
	: mProps({ name, 0, 0, 0 })
{
	Submesh* subs = (Submesh*)malloc(sizeof(Submesh) * meshData.size());

	for (int i = 0; i < meshData.size(); i++)
	{
		new(subs + i) Submesh(meshData[i]);

		mProps.PolyCount += meshData[i].PolyCount;
		mProps.VertexCount += meshData[i].VertexBufferData.size();
		mProps.IndexCount += meshData[i].IndexBufferData.size();

		mSubmeshes.push_back(subs + i);
	}
}

Mesh::~Mesh()
{
	for (Submesh* sub : mSubmeshes)
	{
		sub->~Submesh();
		free(sub);
	}
}

void Mesh::SetTextureForAllSubmeshes(Texture* texture)
{
	for (auto sub : mSubmeshes)
	{
		sub->Texture = texture;
	}
}