#pragma once

#include "Defines.h"

#include <string.h>
#include <cstdint>

#include "Asset.h"

namespace hro
{
    enum class TextureFormat : uint8_t
    {
        Unknown, 
        R8,
        RGB8,
        RGBA8
    };

	TextureFormat ParseTextureFormat(const char* f);

    struct TextureInfo
    {
        TextureFormat format; // Format of the texture
        uint64_t size; // Size of the texture in bytes
        CompressionMode compression_mode;
        uint32_t pixel_size[2]; // width, height 
        std::string original_file_path;
    };

    class HAPI Texture : public Asset
    {
    public:
        Texture() = default;
        Texture(TextureInfo& texture_info); // Texture from texture info

        virtual ~Texture();

        virtual void Pack(void* raw_data, size_t raw_data_size) override;
        virtual void Unpack(void* dst_buffer) override;

        const TextureInfo& Info() const {return info;}
    protected:
        virtual bool ParseInfo(const char* meta_data) override;
    private:
        TextureInfo info = {};
    };
}