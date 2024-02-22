#pragma once

#include "Defines.h"

#include <string>
#include <vector>

namespace hro
{
    enum class CompressionMode : uint8_t
    {
        None, 
        LZ4
    };

    CompressionMode ParseCompressionMode(const char *f);

    const int ASSET_TYPE_SIZE = 4;
    class HAPI Asset
    {
    public:
        char type[ASSET_TYPE_SIZE];
        uint32_t version;
        std::string json_meta_data;
        std::vector<char> packed_data;

        Asset() = default;
        virtual ~Asset() = default;

        // Packs raw data into asset
        virtual void Pack(void* raw_data, size_t raw_data_size) = 0;

        // Unpacks asset raw data into dst buffer
        virtual void Unpack(void* dst_buffer) = 0;
    public:
        // Returns whether or not an asset was loaded successfully 
        bool Load(const char* path);

        // Returns whether or not an asset was saved successfully 
        bool Save(const char* path);

        virtual bool ParseInfo(const char* meta_data) = 0;
    protected:
        bool packed;
    };
}