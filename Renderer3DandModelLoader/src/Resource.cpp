#include "Resource.h"

#include "vendor/tinyobj/tiny_obj_loader.h"
#include "vendor/stb/stb_image.h"

String GetFilenameFromPath(const String& path)
{
    int offset = path.find_last_of('\\');

    if (offset != std::string::npos)
    {
        return path.substr(offset + 1, path.size() - offset);
    }
    
    return path;
}

Texture* LoadTextureFromFile(const String& path)
{
	int width, height, channels;

	unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, DEFAULT_TEX_CHANNELS);

    if (!data)
    {
        ShowAlert("stbi loading error!");
        return nullptr;
    }

	TextureData tData = { data, GetFilenameFromPath(path), width, height, channels };

	Texture* res = new Texture(tData);

	stbi_image_free(data);

	return res;
}

Mesh* LoadMeshFromFile(const String& path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str()))
    {
        ShowAlert(String("tinyobj error: ").append(err));
        return nullptr;
    }

    if (!warn.empty())
    {
        ShowAlert(String("tinyobj warning: ").append(warn));
    }

    std::vector<MeshData> submeshes;

    for (const auto& shape : shapes)
    {
        MeshData md;
        md.Name = shape.name;
        md.PolyCount = shape.mesh.indices.size() / 3; // is this correct ?

        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex = {};

            // vertices
            vertex.Location =
            {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2],
                1.0f
            };

            // texture coords
            if (index.texcoord_index != -1)
            {
                vertex.TextureCoords =
                {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            // normals
            if (index.normal_index != -1)
            {
                vertex.Normal =
                {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }

            // color
            vertex.Color = { 1.0f, 1.0f, 1.0f, 1.0f };

            md.VertexBufferData.push_back(vertex);
            md.IndexBufferData.push_back(md.IndexBufferData.size());
        }

        submeshes.push_back(md);
    }

    return new Mesh(GetFilenameFromPath(path), submeshes);
}