#pragma once

#include "Core.h"

struct MeshData
{
    String Name;
    std::vector<Vertex> VertexBufferData;
    std::vector<uint32_t> IndexBufferData;
    uint32_t PolyCount;
};

class Mesh
{
public:
    Mesh(MeshData* meshData);
    ~Mesh();

    inline struct ID3D11Buffer* GetVertexBuffer() const { return m_VertexBuffer; }
    inline struct ID3D11Buffer* GetIndexBuffer() const { return m_IndexBuffer; }
    inline class Texture* GetTexture() const { return m_Texture; }
    inline const String& GetName() const { return m_Name; }
    inline uint32_t GetVertexBufferCount() const { return m_VertexBufferCount; }
    inline uint32_t GetIndexBufferCount() const { return m_IndexBufferCount; }
    inline uint32_t GetPolyCount() const { return m_PolyCount; }

    void SetTexture(class Texture* texture)
    {
        m_Texture = texture;
    }

private:
    struct ID3D11Buffer* m_VertexBuffer;
    struct ID3D11Buffer* m_IndexBuffer;
    class Texture* m_Texture;
    String m_Name;
    uint32_t m_VertexBufferCount;
    uint32_t m_IndexBufferCount;
    uint32_t m_PolyCount;
};
