cmake_minimum_required(VERSION 3.0)
project(stb_image)

add_library(${PROJECT_NAME} STATIC stb_image.h stb_image.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC "./")
