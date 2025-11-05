#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <gl/attribs.hpp>
#include <gl/buffer.hpp>
#include <gl/cubeMap.hpp>
#include <gl/framebuffer.hpp>
#include <gl/shaders.hpp>
#include <gl/texture.hpp>
#include <gl/vao.hpp>

#ifndef NDEBUG
namespace gl {
  void GLAPIENTRY debugMessageCallback(GLenum source, GLenum type, GLuint id,
                                       GLenum severity, GLsizei length,
                                       const GLchar* message,
                                       const void* userParam);
}
#endif
