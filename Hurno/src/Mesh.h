#ifndef MESH_H
#define MESH_H

#pragma once

#include "Asset.h"

namespace hro
{
    // Vertex format all in f32
    struct HAPI Vertex_F32_PNCV 
    {
        float position[3];
        float color[3];
        float normal[3];
        float uv[2];
    };

    enum class VertexFormat : uint8_t
    {
        Uknown, 
        F32_PNCV
    };

    enum class IndexFormat : uint8_t
    {
        Uknown, 
        UINT32
    };

    struct MeshInfo
    {
        VertexFormat vertex_format; 
        uint64_t vertex_buffer_size; 

        IndexFormat index_format; 
        uint64_t index_buffer_size; 

        std::string name;

        float model_matrix[16];
        uint32_t material_id;

        CompressionMode compression_mode;
        std::string original_file_path;
    };

    struct HAPI Mesh 
    {

    };
}

#endif