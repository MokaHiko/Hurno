#include "Texture.h"

#include <nlohmann/json.hpp>
#include <lz4.h>

namespace hro
{
	TextureFormat ParseTextureFormat(const char* f)
	{
		if (strcmp(f, "RGBA8") == 0)
			return TextureFormat::RGBA8;
		else if (strcmp(f, "RGB8") == 0)
			return TextureFormat::RGB8;
		else if (strcmp(f, "R8") == 0)
			return TextureFormat::R8;
		else
			return TextureFormat::Unknown;
	}

	Texture::Texture(TextureInfo& texture_info)
		:info(texture_info)
	{
	}

	Texture::~Texture()
	{
	}

	void Texture::Pack(void* raw_data, size_t raw_data_size)
	{
		static const char* format_look_up[] =
		{
			"Unknown",
			"R8",
			"RGB8",
			"RGBA8",
		};

		type[0] = 'T';
		type[1] = 'E';
		type[2] = 'X';

		type[3] = 'I';

		version = 1;

		nlohmann::json meta_data = {};
		meta_data["format"] = format_look_up[(int)info.format];
		meta_data["width"] = info.pixel_size[0];
		meta_data["height"] = info.pixel_size[1];
		meta_data["size"] = info.size;
		meta_data["original_file_path"] = info.original_file_path;

		int compressed_bound = LZ4_compressBound(info.size);
		packed_data.resize(compressed_bound);
		int compressed_size = LZ4_compress_default((const char*)raw_data, packed_data.data(), raw_data_size, compressed_bound);
		packed_data.resize(compressed_size);

		meta_data["compression_mode"] = "LZ4";

		json_meta_data = meta_data.dump();
	}

	void Texture::Unpack(void* dst_buffer)
	{
		if (info.compression_mode == CompressionMode::LZ4)
		{
			LZ4_decompress_safe(packed_data.data(), (char*)dst_buffer, packed_data.size(), info.size);
		}
		else
		{
			// NO compression
			memcpy(dst_buffer, packed_data.data(), packed_data.size());
		}
	}

	bool Texture::ParseInfo(const char* meta_data) 
	{ 
		nlohmann::json texture_metadata = nlohmann::json::parse(json_meta_data);

		std::string format_string = texture_metadata["format"];
		info.format = ParseTextureFormat(format_string.c_str());

		info.pixel_size[0] = texture_metadata["width"];
		info.pixel_size[1] = texture_metadata["height"];
		info.size = texture_metadata["size"];
		info.original_file_path = texture_metadata["original_file_path"];

		std::string compression_string = texture_metadata["compression_mode"];
		info.compression_mode = ParseCompressionMode(compression_string.c_str());

		return true;
	}

} // namespace hro