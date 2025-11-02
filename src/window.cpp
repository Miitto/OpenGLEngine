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

  Window::Window(int width, int height, const char* title, bool makeCurrent) {
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (makeCurrent) {
      this->makeCurrent();
    }
  }

  Window::~Window() { glfwDestroyWindow(window); }

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
    glfwGetWindowSize(window, &s.width, &s.height);
    return s;
  }

} // namespace engine
