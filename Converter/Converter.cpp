#include <Hurno.h>
#include <SkeletalMesh.h>

#include <filesystem>
#include <iostream>

static void PrintFullPath(char *partialPath)
{
	char full[_MAX_PATH];
	if (_fullpath(full, partialPath, _MAX_PATH) != NULL)
		printf("Full path is: %s\n", full);
	else
		printf("Invalid path\n");
}

int main(int argc, char* argv[])
{
	std::filesystem::path conversion_path = std::filesystem::current_path();

	bool force_reconvert = false;
	if (argc > 1)
	{
		if (strcmp(argv[1], "--force") == 0)
		{
			printf("Full reconvert!\n");
			force_reconvert = true;
		}
	}
	if (argc > 2)
	{
		conversion_path = std::filesystem::path(argv[2]);
	}

	using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
	std::error_code e;
	for (const auto &dir_entry: recursive_directory_iterator(std::filesystem::current_path()))
	{
		if(dir_entry.is_directory(e))
		{
		}
		else if(dir_entry.is_regular_file(e))
		{
			auto file_path = std::filesystem::path(dir_entry);
			auto extension = file_path.extension();

			if (file_path.extension() == ".yo")
			{
				continue;
			}

			if(file_path.extension() == ".png" || file_path.extension() == ".jpg" || file_path.extension() == ".tga")
			{
				const std::string in_path = file_path.generic_string();
				const std::string out_path = file_path.replace_extension(".yo").generic_string();

				if (std::filesystem::exists(out_path) && !force_reconvert) 
				{
                    continue;
                }

				std::cout << "Converting: " << file_path << std::endl;
				hro::ConvertTexture(in_path.c_str(), out_path.c_str());
			}

			if(file_path.extension() == ".obj")
			{
				const std::string in_path = file_path.generic_string();
				const std::string out_path = file_path.replace_extension(".yo").generic_string();

				if (std::filesystem::exists(out_path) && !force_reconvert) 
				{
                    continue;
                }

				std::cout << "Converting: " << file_path << std::endl;
				hro::ConvertModel(in_path.c_str(), out_path.c_str(), true);
			}
		}
		else if(e)
		{
			std::cerr << "Error in regular file!" << std::endl;
		}
	}

	// hro::SkeletalMesh sk_mesh = {};
	// hro::AssetInfo sk_info = {};
	// sk_mesh.Load("assets/skeletal_meshes/test.yskmesh");

	// sk_mesh.ParseInfo(&sk_info);
	// sk_mesh.Unpack(&sk_info, nullptr);

	return 0;
}