#include "engine/glLoader.hpp"

#include "gl/gl.hpp"
#include "logger.hpp"

namespace engine {
  bool GlLoader::loadedGl = false;

  bool GlLoader::loadGl() {
    if (loadedGl) {
      engine::Logger::warn("Attempted to load OpenGL multiple times");
      return false;
    }

    int version =
        gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

    if (version != 0) {
      loadedGl = true;
      Logger::info("Loaded OpenGL version: {}.{}", GLVersion.major,
                   GLVersion.minor);
      gl::initAttribs();
    } else {
      engine::Logger::error("Failed to load OpenGL");
      return false;
    }

#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    // Allows to breakpoint on GL errors and have the callstack be correct
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(gl::debugMessageCallback, 0);
    engine::Logger::info("Attached debug message callback");
#endif

    return true;
  }

  void GlLoader::glMajor(int major) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
  }

  void GlLoader::glMinor(int minor) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
  }
} // namespace engine
