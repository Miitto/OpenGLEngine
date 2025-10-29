cmake_minimum_required(VERSION 3.15..4.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(OpenGLEngine VERSION 0.1.0 LANGUAGES CXX)

add_subdirectory(gl)
add_subdirectory(logger)
add_subdirectory(src)
add_subdirectory(include)

target_link_libraries(${PROJECT_NAME} PUBLIC gl::gl logger::logger)
