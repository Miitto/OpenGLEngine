#pragma once

#include <glad/glad.h>

namespace gl {
  extern GLint UNIFORM_BUFFER_OFFSET_ALIGNMENT;
  extern GLint TEXTURE_MAX_LEVEL;

  /// <summary>
  /// Initializes global attribute-related values.
  /// Should be called once after OpenGL context creation.
  /// </summary>
  void initAttribs();
} // namespace gl