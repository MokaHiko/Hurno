#pragma once

#include "Defines.h"

#include "Asset.h"
#include "Mesh.h"

#include "Material.h"

namespace hro
{
    struct HAPI ModelInfo
    {
        ModelInfo()
        {
            //printf("ModelInfo Created!\n");
        }
        uint64_t vertex_buffer_size; // size of vertex buffer in bytes
        uint64_t index_buffer_size;  // size of vertex buffer in bytes

        CompressionMode compression_mode;

        uint64_t mesh_count;
        std::vector<MeshInfo> mesh_infos;

        uint64_t material_count;
        std::vector<Material> materials;

        std::string original_file_path;
    };

    /*
        Models hold meta data and materials as well as the compressed vertex and index data of a mesh
    */
    class HAPI Model : public Asset
    {
    public:
        // Texture from texture info
        Model(ModelInfo&& model_info);
        Model() = default;
        ~Model() = default;

        virtual void Pack(void* raw_data, size_t raw_data_size) override;
        virtual void Unpack(void* dst_buffer) override;

        const ModelInfo& Info() const { return info; }
    protected:
        virtual bool ParseInfo(const char* meta_data) override;
    private:
        ModelInfo info;
    };
}
