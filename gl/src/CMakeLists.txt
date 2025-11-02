target_sources(${PROJECT_NAME}
  PRIVATE
  FILE_SET privateHeaders
  TYPE HEADERS

  PRIVATE
    buffer.cpp
    logger.cpp
    vao.cpp
    shaders.cpp
 "attribs.cpp" "debug.cpp" "texture.cpp")