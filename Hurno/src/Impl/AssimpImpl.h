#pragma once

#include <assimp/scene.h>
#include <filesystem>

#include "Mesh.h"
#include "Material.h"

// glm::mat4 AssimpToGLMMat4(const aiMatrix4x4& from)
// {
//     glm::mat4 to(1.0f);

//     //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
//     to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
//     to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
//     to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
//     to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
//     return to;
// }

void TranposeMat4x4(float* matrix)
{
};

template<typename VertexFormat>
class AssimpLoader
{
public:
    uint64_t index_count() const { return indices.size(); }
    uint64_t vertex_count() const { return vertices.size(); }

    uint64_t mesh_count() const { return mesh_infos.size(); }
    uint64_t material_count() const { return materials.size(); }

    VertexFormat* vertex_data() { return vertices.data(); }
    uint32_t* index_data() { return indices.data(); }

    hro::Material* material_data() { return materials.data(); }

    std::vector<hro::MeshInfo> mesh_infos = {};
    std::vector<hro::Material> materials = {};

    ~AssimpLoader() {}

    void load_scene(const aiScene* scene)
    {
        uint32_t mesh_count = scene->mNumMeshes;
        uint32_t material_count = scene->mNumMaterials;

        process_materials(scene);

        const aiMatrix4x4 identity = {};
        process_node_recursive(scene->mRootNode, identity, scene);
    }
private:
    void process_node_recursive(aiNode* node, const aiMatrix4x4& parent_global_matrix, const aiScene* scene)
    {
        // Load Meshes
        const aiMatrix4x4 global_node_model_matrix = node->mTransformation * parent_global_matrix;
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            process_mesh(mesh, global_node_model_matrix, scene);
        }

        // Call for children
        for (uint32_t i = 0; i < node->mNumChildren; i++)
        {
            process_node_recursive(node->mChildren[i], global_node_model_matrix, scene);
        }
    }

    void process_mesh(aiMesh* mesh, const aiMatrix4x4& global_node_model_matrix, const aiScene* scene)
    {
        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            aiVector3D position = mesh->mVertices[i];
            aiVector3D normal = mesh->mNormals[i];

            VertexFormat vertex = {};

            vertex.position[0] = position.x;
            vertex.position[1] = position.y;
            vertex.position[2] = position.z;

            vertex.normal[0] = normal.x;
            vertex.normal[1] = normal.y;

            if (mesh->HasTextureCoords(0))
            {
                aiVector3D uv = mesh->mTextureCoords[0][i];
                vertex.uv[0] = uv.x;
                vertex.uv[1] = uv.y;
            }
            else
            {
                vertex.uv[0] = 0;
                vertex.uv[1] = 0;
            }

            vertices.push_back(vertex);
        }

        int mesh_index_count = 0;
        for (uint32_t i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];

            for (uint32_t j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
                mesh_index_count++;
            }
        }

        hro::MeshInfo mesh_info = {};
        // TODO: Suppport different vertex formats
        mesh_info.vertex_format = hro::VertexFormat::F32_PNCV;
        mesh_info.vertex_buffer_size = mesh->mNumVertices * sizeof(VertexFormat);

        mesh_info.index_format = hro::IndexFormat::UINT32;
        mesh_info.index_buffer_size = mesh_index_count * sizeof(uint32_t);

        mesh_info.name = mesh->mName.data;
        mesh_info.material_id = mesh->mMaterialIndex;

        float* data = (float*)(&global_node_model_matrix);
        for(int i = 0; i < 16; i++)
        {
            mesh_info.model_matrix[i] = data[i]; 
        }

        mesh_infos.push_back(mesh_info);
    }

    void process_materials(const aiScene* scene)
    {
        for (uint32_t i = 0; i < scene->mNumMaterials; i++)
        {
            hro::Material material = {};

            const aiMaterial* ai_material = scene->mMaterials[i];
            auto path= std::filesystem::path(ai_material->GetName().data);
            path.replace_extension();
            std::string name = path.string();

            // Check if material name already taken
            bool processed = false;
            for (hro::Material& m : materials)
            {
                if (m.name.data() == name)
                {
                    processed = true;
                    break;
                }
            }

            if (processed)
            {
                std::string suff = "_" + std::to_string(i);
                name = name + suff;
            }

            // Colors
            aiColor3D ambient_color(1.0);
            if (ai_material->Get(AI_MATKEY_COLOR_AMBIENT, ambient_color) == AI_SUCCESS)
            {
                material.ambient_color[0] = ambient_color.r;
                material.ambient_color[1] = ambient_color.g;
                material.ambient_color[2] = ambient_color.b;
            }

            aiColor3D diffuse_color(1.0);
            if (ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color) == AI_SUCCESS)
            {
                material.diffuse_color[0] = diffuse_color.r;
                material.diffuse_color[1] = diffuse_color.g;
                material.diffuse_color[2] = diffuse_color.b;
            }

            aiColor3D specular_color(1.0);
            if (ai_material->Get(AI_MATKEY_COLOR_SPECULAR, specular_color) == AI_SUCCESS)
            {
                material.specular_color[0] = specular_color.r;
                material.specular_color[1] = specular_color.g;
                material.specular_color[2] = specular_color.b;
            }

            // Textures
            material.diffuse_texture_path = convert_material_texture(ai_material, aiTextureType_DIFFUSE, scene);
            material.specular_texture_path = convert_material_texture(ai_material, aiTextureType_SPECULAR, scene);

            material.name = name;
            materials.push_back(material);
        }
    }

private:
    // Returns converted texture path
    std::string convert_material_texture(const aiMaterial* material, aiTextureType type, const aiScene* scene)
    {
        if (material->GetTextureCount(type) <= 0)
        {
            return "";
        }

        aiString ai_path;
        if (material->GetTexture(type, 0, &ai_path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
        {
            const char* path = ai_path.data;
            std::filesystem::path output_path = std::filesystem::path(path).filename();
            output_path.replace_extension("yo");

            const aiTexture* embedded_texture = scene->GetEmbeddedTexture(path);

            if (embedded_texture)
            {
                uint32_t nChannels = 4;
                uint32_t buffer_size = embedded_texture->mWidth;
                if (!hro::ConvertTexture(embedded_texture->pcData, buffer_size, std::filesystem::path(path).string().c_str(), output_path.string().c_str()))
                {
                    return "";
                }
            }
            else
            {
                // TODO: Relative path
                // hro::ConvertTexture(std::filesystem::path(path).string().c_str(), output_path.string().c_str());
            }

            return output_path.string();
        }

        return "";
    }
private:
    std::vector<VertexFormat> vertices;
    std::vector<uint32_t> indices;
};