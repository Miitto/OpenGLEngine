#pragma once

#include "gl/gl.hpp"

namespace gl {
  extern GLint UNIFORM_BUFFER_OFFSET_ALIGNMENT;

  /// <summary>
  /// Initializes global attribute-related values.
  /// Should be called once after OpenGL context creation.
  /// </summary>
  void initAttribs();
} // namespace gl