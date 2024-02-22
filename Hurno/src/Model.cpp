#include "Model.h"

#include <nlohmann/json.hpp>
#include <lz4.h>

hro::Model::Model(ModelInfo&& model_info)
	:info(std::move(model_info))
{

}

void hro::Model::Pack(void* raw_data, size_t raw_data_size)
{
	type[0] = 'M';
	type[1] = 'O';
	type[2] = 'D';
	type[3] = 'L';

	version = 1;

	nlohmann::json meta_data = {};
	meta_data["mesh_count"] = info.mesh_count;
	meta_data["vertex_buffer_size"] = info.vertex_buffer_size;
	meta_data["index_buffer_size"] = info.index_buffer_size;
	meta_data["original_file_path"] = info.original_file_path;

	/*
		Models are packed sequentially into the asset file
	*/
	meta_data["mesh_count"] = info.mesh_count;
	meta_data["mesh_infos"] = nlohmann::json::array();
	for (int i = 0; i < info.mesh_count; i++)
	{
		const MeshInfo& mesh_info = info.mesh_infos[i];

		nlohmann::json mesh_info_meta_data;
		mesh_info_meta_data["name"] = mesh_info.name;

		mesh_info_meta_data["vertex_format"] = mesh_info.vertex_format;
		mesh_info_meta_data["vertex_buffer_size"] = mesh_info.vertex_buffer_size;

		mesh_info_meta_data["index_format"] = mesh_info.index_format;
		mesh_info_meta_data["index_buffer_size"] = mesh_info.index_buffer_size;

		mesh_info_meta_data["material_id"] = mesh_info.material_id;
		mesh_info_meta_data["model_matrix"] = nlohmann::json::array();
		for (int i = 0; i < 16; i++)
		{
			mesh_info_meta_data["model_matrix"].push_back(mesh_info.model_matrix[i]);
		}

		meta_data["mesh_infos"].push_back(mesh_info_meta_data);
	}

	meta_data["material_count"] = info.material_count;
	meta_data["materials"] = nlohmann::json::array();
	for (int i = 0; i < info.material_count; i++)
	{
		const Material& material = info.materials[i];

		nlohmann::json material_meta_data;
		material_meta_data["name"] = material.name;

		const float* ambient = material.ambient_color;
		material_meta_data["ambient_color"] = { ambient[0], ambient[1], ambient[2] };

		const float* diffuse = material.diffuse_color;
		material_meta_data["diffuse_color"] = { diffuse[0], diffuse[1], diffuse[2] };

		const float* specular = material.specular_color;
		material_meta_data["specular_color"] = { specular[0], specular[1], specular[2] };

		material_meta_data["diffuse_texture_path"] = info.materials[i].diffuse_texture_path.c_str();
		material_meta_data["specular_texture_path"] = info.materials[i].specular_texture_path.c_str();
		material_meta_data["normal_texture_path"] = info.materials[i].normal_texture_path.c_str();

		meta_data["materials"].push_back(material_meta_data);
	}

	int compressed_bound = LZ4_compressBound(info.vertex_buffer_size + info.vertex_buffer_size);
	packed_data.resize(compressed_bound);
	int compressed_size = LZ4_compress_default((const char*)raw_data, packed_data.data(), raw_data_size, compressed_bound);
	packed_data.resize(compressed_size);

	meta_data["compression_mode"] = "LZ4";
	json_meta_data = meta_data.dump();
}

void hro::Model::Unpack(void* dst_buffer)
{
	// TODO: Malloc the data for user
	if (info.compression_mode == CompressionMode::LZ4)
	{
		LZ4_decompress_safe(packed_data.data(), (char*)dst_buffer, packed_data.size(), info.index_buffer_size + info.vertex_buffer_size);
	}
	else
	{
		// NO compression
		memcpy(dst_buffer, packed_data.data(), packed_data.size());
	}
}

bool hro::Model::ParseInfo(const char* meta_data)
{
	nlohmann::json metadata = nlohmann::json::parse(json_meta_data);
	info.original_file_path = metadata["original_file_path"];

	info.material_count = metadata["material_count"];
	nlohmann::json materials = metadata["materials"];

	info.compression_mode = CompressionMode::LZ4;

	int material_counter = 0;
	info.materials.resize(info.material_count);
	for (nlohmann::json::iterator it = materials.begin(); it != materials.end(); it++)
	{
		Material material = {};
		material.name = (*it)["name"];

		nlohmann::json ambient = (*it)["ambient_color"];
		material.ambient_color[0] = ambient[0];
		material.ambient_color[1] = ambient[1];
		material.ambient_color[2] = ambient[2];

		nlohmann::json diffuse = (*it)["diffuse_color"];
		material.diffuse_color[0] = diffuse[0];
		material.diffuse_color[1] = diffuse[1];
		material.diffuse_color[2] = diffuse[2];

		nlohmann::json specular = (*it)["specular_color"];
		material.specular_color[0] = specular[0];
		material.specular_color[1] = specular[1];
		material.specular_color[2] = specular[2];

		material.diffuse_texture_path = (*it)["diffuse_texture_path"];
		material.specular_texture_path = (*it)["specular_texture_path"];

		info.materials[material_counter++] = material;
	}

	uint64_t offset = 0;
	info.mesh_count = metadata["mesh_count"];
	nlohmann::json mesh_infos = metadata["mesh_infos"];

	int ctr = 0;
	info.mesh_infos.resize(info.mesh_count);

	for (nlohmann::json::iterator it = mesh_infos.begin(); it != mesh_infos.end(); it++)
	{
		// Update mesh info
		info.mesh_infos[ctr].vertex_buffer_size = (*it)["vertex_buffer_size"];
		info.mesh_infos[ctr].index_buffer_size = (*it)["index_buffer_size"];
		info.mesh_infos[ctr].name = (*it)["name"];
		info.mesh_infos[ctr].material_id = (*it)["material_id"];

		// TODO: Pack better
		nlohmann::json model_matrix = (*it)["model_matrix"];
		for (int i = 0; i < 16; i++)
		{
			info.mesh_infos[ctr].model_matrix[i] = model_matrix[i];
		}

		// Update global info
		info.mesh_infos[ctr].vertex_format = (VertexFormat)((*it)["vertex_format"]);
		info.vertex_buffer_size += (uint64_t)((*it)["vertex_buffer_size"]);

		info.mesh_infos[ctr].index_format = (IndexFormat)((*it)["index_format"]);
		info.index_buffer_size += (uint64_t)((*it)["index_buffer_size"]);

		ctr++;
	}

	return true;
}

