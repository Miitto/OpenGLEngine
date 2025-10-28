cmake_minimum_required(VERSION 3.15..4.0)

project(imgui VERSION 3.92.4 LANGUAGES CXX)

add_library(imgui)
add_library(imgui::imgui ALIAS imgui)

include(glfw)
LINK_GLFW(imgui PRIVATE)

target_sources(imgui
 PRIVATE
 imgui/imgui.cpp
 imgui/imgui_demo.cpp
 imgui/imgui_draw.cpp
 imgui/imgui_tables.cpp
 imgui/imgui_widgets.cpp
 imgui/backends/imgui_impl_glfw.cpp
 imgui/backends/imgui_impl_opengl3.cpp
 PUBLIC
 FILE_SET internalHeaders
 TYPE HEADERS
 BASE_DIRS
 imgui
 FILES
 imgui/imgui.h
 PUBLIC
 FILE_SET HEADERS
)

set_source_files_properties(
  imgui/imgui.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/backends/imgui_impl_glfw.cpp
  imgui/backends/imgui_impl_opengl3.cpp
  PROPERTIES
  SKIP_LINTING ON
)

# Disable warnings for third-party code
target_compile_options(imgui PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W0>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-w>
)
