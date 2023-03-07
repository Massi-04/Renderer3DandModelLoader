#pragma once

#include "Core.h"

struct Vertex
{
    Vertex()
    {
        Location = { 0.0f, 0.0f, 0.0f, 0.0f };
        Color = { 0.0f, 0.0f, 0.0f, 0.0f };
        Normal = { 0.0f, 0.0f, 0.0f };
        TextureCoords = { 0.0f, 0.0f };
    }

    Vertex(Vec4 loc, Vec4 col, Vec3 normal, Vec2 texCoord)
    {
        Location = loc;
        Color = col;
        Normal = normal;
        TextureCoords = texCoord;
    }

    Vec4 Location;
    Vec4 Color;
    Vec3 Normal;
    Vec2 TextureCoords;
};

struct MeshData
{
    String Name;
    std::vector<Vertex> VertexBufferData;
    std::vector<uint32_t> IndexBufferData;
    uint32_t PolyCount;
};

struct MeshProps
{
    String Name;
    uint32_t PolyCount;
    uint32_t VertexCount;
    uint32_t IndexCount;
};

class Submesh
{
public:
    Submesh(const MeshData& meshData);
    ~Submesh();

public:
    const MeshProps& GetProps() const { return mProps; }
    struct ID3D11Buffer* GetVertexBuffer() const { return mVertexBuffer; }
    struct ID3D11Buffer* GetIndexBuffer() const { return mIndexBuffer; }

public:
    class Texture* Texture;

private:
    MeshProps mProps;
    struct ID3D11Buffer* mVertexBuffer;
    struct ID3D11Buffer* mIndexBuffer;
};

class Mesh
{
public:
    Mesh(const String& name, const std::vector<MeshData>& meshData);
    ~Mesh();

public:
    const MeshProps& GetProps() const { return mProps; }
    const std::vector<Submesh*>& GetSubmeshes() const { return mSubmeshes; }
    void SetTextureForAllSubmeshes(class Texture* texture);
    class Texture* GetTexture(uint32_t submeshIndex = 0) { return mSubmeshes[submeshIndex]->Texture; }

private:
    MeshProps mProps;
    std::vector<Submesh*> mSubmeshes;
};
