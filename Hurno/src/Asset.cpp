#include "Asset.h"

#include <fstream>

namespace hro
{
	bool Asset::Load(const char* path)
	{
		std::ifstream file;
		file.open(path, std::ios::binary);

		if (!file.is_open())
		{
			return false;
		}

		file.seekg(0);

		file.read(type, ASSET_TYPE_SIZE);
		file.read((char*)&version, sizeof(uint32_t));

		uint32_t meta_data_size = 0;
		file.read((char*)&meta_data_size, sizeof(uint32_t));

		uint32_t packed_data_size = 0;
		file.read((char*)&packed_data_size, sizeof(uint32_t));

		json_meta_data.resize(static_cast<size_t>(meta_data_size));
		file.read((char*)json_meta_data.data(), meta_data_size);

		packed_data.resize(static_cast<size_t>(packed_data_size));
		file.read(packed_data.data(), packed_data_size);

		file.close();

		return ParseInfo(json_meta_data.c_str());
	}

	bool Asset::Save(const char* path)
	{
		std::ofstream file;
		file.open(path, std::ios::binary | std::ios::out);

		if (!file.is_open())
		{
			return false;
		}

		file.write(type, ASSET_TYPE_SIZE);
		file.write((const char*)&version, sizeof(uint32_t));

		// Meta data size
		uint32_t meta_data_size = json_meta_data.size();
		file.write((const char*)&meta_data_size, sizeof(uint32_t));

		// Packed data size
		uint32_t packed_data_size = packed_data.size();
		file.write((const char*)&packed_data_size, sizeof(uint32_t));

		file.write(json_meta_data.data(), meta_data_size);
		file.write(packed_data.data(), packed_data_size);

		file.close();

		return true;
	}

	CompressionMode ParseCompressionMode(const char* f) 
	{
        if (strcmp(f, "LZ4") == 0)
            return CompressionMode::LZ4;
        else
            return CompressionMode::None;
	}
} // namespace hro
