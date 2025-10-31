#include "engine/image.hpp"

namespace engine {
  bool Image::inited = false;

  void Image::ensureInit() {
    if (!inited) {
      stbi_set_flip_vertically_on_load(true);
      inited = true;
    }
  }
} // namespace engine
