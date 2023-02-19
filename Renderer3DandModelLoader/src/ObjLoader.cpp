#include "ObjLoader.h"
#include "vendor/tinyobj/tiny_obj_loader.h"

std::vector<MeshData> LoadObj(const char* filePath)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filePath))
    {
        ShowAlert(String("tinyobj error: ").append(err));
        return {};
    }

    if (!warn.empty())
    {
        ShowAlert(String("tinyobj warning: ").append(warn));
    }

    std::vector<MeshData> res;

    for (const auto& shape : shapes) 
    {
        MeshData md;
        md.Name = shape.name;
        md.PolyCount = shape.mesh.indices.size() / 3; // is this correct ?

        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex = {};

            vertex.Location =
            {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2],
                1.0f
            };

            if (index.texcoord_index != -1)
            {
                vertex.TextureCoords =
                {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            vertex.Color = { 1.0f, 1.0f, 1.0f, 1.0f };

            md.VertexBufferData.push_back(vertex);
            md.IndexBufferData.push_back(md.IndexBufferData.size());
        }

        res.push_back(md);
    }

    return res;
}
