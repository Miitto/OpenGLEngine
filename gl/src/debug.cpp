#include "gl/gl.hpp"

#include "logger.hpp"

namespace gl {
  void GLAPIENTRY debugMessageCallback(GLenum source, GLenum type, GLuint id,
                                       GLenum severity, GLsizei length,
                                       const GLchar* message,
                                       const void* userParam) {
    (void)id;
    (void)length;
    (void)userParam;

    const char* source_str = "";
    switch (source) {
    case GL_DEBUG_SOURCE_API:
      source_str = "API";
      break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      source_str = "WINDOW_SYSTEM";
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      source_str = "SHADER_COMPILER";
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      source_str = "THIRD_PARTY";
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      source_str = "APPLICATION";
      break;
    case GL_DEBUG_SOURCE_OTHER:
      source_str = "OTHER";
      break;
    }

    const char* type_str = "";

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
      type_str = "ERROR";
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      type_str = "DEPRECATED_BEHAVIOR";
      break;

    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      type_str = "UNDEFINED_BEHAVIOR";
      break;

    case GL_DEBUG_TYPE_PORTABILITY:
      type_str = "PORTABILITY";
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      type_str = "PERFORMANCE";
      break;
    case GL_DEBUG_TYPE_MARKER:
      type_str = "MARKER";
      break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
      type_str = "PUSH_GROUP";
      break;
    case GL_DEBUG_TYPE_POP_GROUP:
      type_str = "POP_GROUP";
      break;
    case GL_DEBUG_TYPE_OTHER:
      type_str = "OTHER";
      return;
      break;
    }

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
      gl::Logger::error("GL {} ERROR: ({}) | {}", source_str, type_str,
                        message);
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      gl::Logger::warn("GL {} WARNING: ({}) | {}", source_str, type_str,
                       message);
      break;
    case GL_DEBUG_SEVERITY_LOW:
      gl::Logger::info("GL {} INFO: ({}) | {}", source_str, type_str, message);
      break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      gl::Logger::debug("GL {} DEBUG: ({}) | {}", source_str, type_str,
                        message);
      break;
    default:
      gl::Logger::debug("GL {} UNKNOWN[{}]: ({}) | {}", source_str, severity,
                        type_str, message);
      break;
    }
  }
} // namespace gl
