#include "engine/app.hpp"
#include "imgui/imgui.h"
#include "logger.hpp"

namespace engine {
  App::App(int width, int height, const char title[])
      : window({width, height, title, true}), input(window), gui(window) {
    auto& wm = engine::WindowManager::get();

    if (!wm.isGlLoaded()) {
      Logger::critical("Failed to load OpenGL");
      throw std::runtime_error("Failed to load OpenGL");
    }
  }

  void App::update(const FrameInfo& frame) {
    window.pollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      gui.sleep(10);
      return;
    }
    gui.newFrame();
    input.imGuiWantsMouse(gui.io().WantCaptureMouse);
    input.imGuiWantsKeyboard(gui.io().WantCaptureKeyboard);
  }
} // namespace engine