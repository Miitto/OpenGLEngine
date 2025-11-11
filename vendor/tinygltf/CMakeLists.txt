cmake_minimum_required(VERSION 3.15..4.0)

project(tinygltf LANGUAGES CXX)

add_library(tinygltf)
add_library(tinygltf::tinygltf ALIAS tinygltf)

target_link_libraries(tinygltf PUBLIC stb::stb)

target_sources(tinygltf
  PUBLIC
  FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
  src/tiny_gltf.cpp
)