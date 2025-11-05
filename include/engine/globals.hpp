#pragma once

#include <gl/gl.hpp>

namespace engine::globals {
  /// <summary>
  /// Global dummy VAO for use when no VAO is needed.
  /// </summary>
  extern gl::Vao DUMMY_VAO;

  void initGlobals();
} // namespace engine::globals