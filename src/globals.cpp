#include "engine/globals.hpp"

namespace engine::globals {
  gl::Vao DUMMY_VAO = gl::Vao::uninitialized();

  void initGlobals() {
    DUMMY_VAO = gl::Vao();
    DUMMY_VAO.label("Engine Dummy VAO");
  }
} // namespace engine::globals