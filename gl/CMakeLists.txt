cmake_minimum_required(VERSION 3.15..4.0)

project(gl VERSION 0.1.0 LANGUAGES CXX)

include(FetchContent)

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

message(STATUS "Module path: ${CMAKE_MODULE_PATH}")

include(glfw)
link_glfw(${PROJECT_NAME} PUBLIC)

include(glad)
link_glad(${PROJECT_NAME} PUBLIC)

include(glm)
link_glm(${PROJECT_NAME} PUBLIC)

include(imgui)
LINK_IMGUI(${PROJECT_NAME} PUBLIC)

include(spdlog)
link_spdlog(${PROJECT_NAME} PUBLIC)

target_link_libraries(${PROJECT_NAME} PRIVATE logger::logger)

add_subdirectory(src)
add_subdirectory("include")

target_precompile_headers(${PROJECT_NAME} PUBLIC <glad/glad.h> <GLFW/glfw3.h>)
