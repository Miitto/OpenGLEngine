#include <glad/glad.h>
// Include GLFW after glad (comment to avoid autosort)
#include <GLFW/glfw3.h>

#include "engine/window.hpp"

#include "engine/glLoader.hpp"
#include "logger.hpp"

namespace engine {

  std::optional<std::string> WindowManager::initialize() {
    if (glfwInit()) {
      engine::Logger::info("GLFW initialized");
#ifndef NDEBUG
      glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
    } else {
      return std::string("Failed to initialize GLFW");
    }
    GlLoader::glVersion(4, 6);

    engine::Logger::info("Window Manager initialized");

    return std::nullopt;
  }

  std::function<void()> WindowManager::shutdown() {
    return []() {
      glfwTerminate();
      engine::Logger::info("Window Manager terminated");
    };
  }

  Window::Window(int width, int height, const char* title, bool fullscreen,
                 bool makeCurrent)
      : fullscreened(fullscreen), cachedSize({width, height}) {
    GLFWmonitor* monitor = nullptr;
    if (fullscreen) {
      monitor = glfwGetPrimaryMonitor();
      GLFWvidmode const* mode = glfwGetVideoMode(monitor);
      width = mode->width;
      height = mode->height;

      glfwWindowHint(GLFW_RED_BITS, mode->redBits);
      glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
      glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
      glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
      glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    }

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (makeCurrent) {
      this->makeCurrent();
    }
  }

  Window::~Window() { glfwDestroyWindow(window); }

  void Window::fullscreen(bool enable) {
    fullscreened = enable;
    if (enable) {
      GLFWmonitor* monitor = glfwGetPrimaryMonitor();
      GLFWvidmode const* mode = glfwGetVideoMode(monitor);
      glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height,
                           mode->refreshRate);
    } else {
      GLFWmonitor* monitor = glfwGetPrimaryMonitor();
      GLFWvidmode const* mode = glfwGetVideoMode(monitor);

      auto width = mode->width;
      auto height = mode->height;

      width /= 2;
      height /= 2;

      width -= cachedSize.width / 2;
      height -= cachedSize.height / 2;

      glfwSetWindowMonitor(window, nullptr, width, height, cachedSize.width,
                           cachedSize.height, 0);
    }
  }

  void Window::makeCurrent() const { glfwMakeContextCurrent(window); }

  bool Window::shouldClose() const { return glfwWindowShouldClose(window); }
  void Window::swapBuffers() const { glfwSwapBuffers(window); }
  void Window::pollEvents() { glfwPollEvents(); }

  void Window::setUserPtr(void* ptr) { glfwSetWindowUserPointer(window, ptr); }
  void* Window::getUserPtr(GLFWwindow* window) {
    return glfwGetWindowUserPointer(window);
  }

  Window::Size Window::size() const {
    Size s;
    glfwGetFramebufferSize(window, &s.width, &s.height);

    return s;
  }

} // namespace engine
