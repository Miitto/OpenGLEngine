#include "gl/attribs.hpp"

namespace gl {
  GLint UNIFORM_BUFFER_OFFSET_ALIGNMENT = 0;
  GLint TEXTURE_MAX_LEVEL = 0;

  void initAttribs() {
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
                  &UNIFORM_BUFFER_OFFSET_ALIGNMENT);
    glGetIntegerv(GL_TEXTURE_MAX_LEVEL, &TEXTURE_MAX_LEVEL);
  }
} // namespace gl