#include "Mesh.h"
#include "D3D.h"

Mesh::Mesh(MeshData* meshData)
	: m_VertexBuffer(CreateBuffer(Default, VertexBuffer, None, meshData->VertexBufferData.size() * sizeof(Vertex), meshData->VertexBufferData.data()))
	, m_IndexBuffer(CreateBuffer(Default, IndexBuffer, None, meshData->IndexBufferData.size() * sizeof(uint32_t), meshData->IndexBufferData.data()))
	, m_Name(meshData->Name), m_VertexBufferCount(meshData->VertexBufferData.size()), m_IndexBufferCount(meshData->IndexBufferData.size())
	, m_PolyCount(meshData->PolyCount), m_Texture(nullptr)
{
}

Mesh::~Mesh()
{
	m_VertexBuffer->Release();
	m_IndexBuffer->Release();
}