#include "gl/attribs.hpp"

namespace gl {
  GLint UNIFORM_BUFFER_OFFSET_ALIGNMENT = 0;

  void initAttribs() {
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
                  &UNIFORM_BUFFER_OFFSET_ALIGNMENT);
  }
} // namespace gl