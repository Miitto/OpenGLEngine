add_library(${PROJECT_NAME})
add_library(engine::engine ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PRIVATE
    FILE_SET internalHeaders
    TYPE HEADERS
  PRIVATE
    camera.cpp
    scene_node.cpp
    scene_graph.cpp
    window.cpp
    input.cpp
    logger.cpp
    gui.cpp
    frustum.cpp
    app.cpp
    mesh/mesh_data.cpp
    mesh/mesh.cpp
    mesh/mesh_animation.cpp
    mesh/mesh_material.cpp
    image.cpp
 "glLoader.cpp" "globals.cpp")

include(stb)
link_stb(${PROJECT_NAME} PUBLIC)

include(tinygltf)
link_tinygltf(${PROJECT_NAME} PUBLIC)
