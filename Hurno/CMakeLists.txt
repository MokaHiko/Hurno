cmake_minimum_required(VERSION 3.0.0)
project(Hurno VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_subdirectory(vendor/stb_image)
add_subdirectory(vendor/json)
add_subdirectory(vendor/lz4/build/cmake)

set(CMAKE_CXX_STANDARD 17)
add_library(${PROJECT_NAME} 
						src/Asset.h 
						src/Asset.cpp

						src/Texture.h 
						src/Texture.cpp

						include/Hurno.h 
						include/Hurno.cpp
						)
target_include_directories(${PROJECT_NAME} PUBLIC include src)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json stb_image lz4_static)