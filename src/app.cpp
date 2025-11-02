#include "engine/app.hpp"
#include "engine/glLoader.hpp"
#include "imgui/imgui.h"
#include "logger.hpp"

namespace {
  bool engineInitialized = false;
}

namespace engine {
  bool App::initialized = false;
  std::vector<std::function<void()>> App::pluginShutdowns = {};

  App::App(int width, int height, const char title[])
      : window({width, height, title, true}), input(window), gui(window) {
    if (initialized) {
      Logger::critical("Engine already initialized, exiting");
      throw std::runtime_error("Engine was already initialized");
    }

    auto err = loadPostInitEnginePlugins();
    if (err.has_value()) {
      throw std::runtime_error(err.value());
    }

    initialized = true;
  }

  App::~App() {
    for (auto& shutdown : pluginShutdowns) {
      shutdown();
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

#define LOAD_PLUGIN(T)                                                         \
  {                                                                            \
    auto res = engine::App::registerPlugin<T>();                               \
    if (res.has_value()) {                                                     \
      return res;                                                              \
    }                                                                          \
  }

  std::optional<std::string> loadPreInitEnginePlugins() {
    if (engineInitialized) {
      Logger::warn("Attempted to load engine plugins multiple times");
      return std::nullopt;
    }

    LOAD_PLUGIN(engine::WindowManager)

    engineInitialized = true;
    // Future engine plugins can be loaded here
    return std::nullopt;
  }

  std::optional<std::string> loadPostInitEnginePlugins() {
    LOAD_PLUGIN(engine::GlLoader)

    return std::nullopt;
  }

  namespace __private {
    void logStart() { engine::Logger::info("Entering main loop"); }
  } // namespace __private
} // namespace engine