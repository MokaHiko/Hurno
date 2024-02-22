#include "AssimpImpl.h"

#include <iostream>
#include <chrono>

#include "Hurno.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>

#include "Model.h"

namespace hro
{
	bool ConvertModel(const char* path, const char* out_path)
	{
		using namespace Assimp;

		Importer importer;
		DefaultLogger::create("", Assimp::Logger::NORMAL);
		DefaultLogger::get()->info("Assimp Initalized");

		const aiScene* scene = importer.ReadFile(path,
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_JoinIdenticalVertices |
			aiProcess_FlipUVs | 
			aiProcess_SortByPType
		);

		if (scene == nullptr)
		{
			throw std::runtime_error("Failed to load: " + std::string(path));
			return false;
		}

		auto start = std::chrono::high_resolution_clock::now();

		AssimpLoader<Vertex_F32_PNCV>* model_loader = new AssimpLoader<Vertex_F32_PNCV>();
		model_loader->load_scene(scene);

		auto end = std::chrono::high_resolution_clock::now();

		auto diff = end - start;
		std::cout << "Model loading took" << std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() / 1000000.0 << "ms" << std::endl;

		ModelInfo info = {};
		info.mesh_count = model_loader->mesh_count();
		info.mesh_infos = model_loader->mesh_infos;

		info.material_count = model_loader->material_count();
		info.materials = model_loader->materials;

		info.vertex_buffer_size = model_loader->vertex_count() * sizeof(Vertex_F32_PNCV);
		info.index_buffer_size = model_loader->index_count() * sizeof(uint32_t);
		info.compression_mode = CompressionMode::LZ4;
		info.original_file_path = std::string(path);

		// Compress model as asset file
		try
		{
			start = std::chrono::high_resolution_clock::now();
			float buffer_size = info.vertex_buffer_size + info.index_buffer_size;
			void* data = malloc(buffer_size);

			memcpy(data, model_loader->vertex_data(), info.vertex_buffer_size);
			memcpy((char*)data + info.vertex_buffer_size, model_loader->index_data(), info.index_buffer_size);

			Model model(std::move(info));
			model.Pack(data, buffer_size);

			end = std::chrono::high_resolution_clock::now();
			diff = end - start;
			std::cout << "Model compression took " << std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() / 1000000.0 << "ms" << std::endl;

			// Clean Up
			delete model_loader;
			DefaultLogger::kill();

			return model.Save(out_path);
		}
		catch (std::exception e)
		{
			DefaultLogger::get()->info(e.what());

			// Clean Up
			delete model_loader;
			DefaultLogger::kill();

			return false;
		}
	}
}