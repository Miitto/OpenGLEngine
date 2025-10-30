#pragma once

#include "engine/gui.hpp"
#include "engine/input.hpp"
#include "engine/scene_graph.hpp"
#include "engine/window.hpp"
#include "frame_info.hpp"
#include <chrono>

namespace engine {

  class App {
  protected:
    engine::Window window;
    engine::Input input;
    engine::gui::Context gui;
    int flags = 0;
    uint32_t frameIndex = 0;

  public:
    enum AppFlags {
      NONE = 0,
      BAIL = 1 << 0,
    };

    App() = delete;
    App(int width, int height, const char title[]);

    virtual void update(const FrameInfo& frame);
    virtual void render(const FrameInfo& frame) = 0;

    void bail() { flags |= BAIL; }
    bool shouldBail() const { return (flags & BAIL) != 0; }
    bool shouldClose() const { return window.shouldClose(); }

    uint32_t getFrameIndex() const { return frameIndex; }

    void postRender() {
      input.frameEnd();
      gui.endFrame();
      window.swapBuffers();

      ++frameIndex;
    }
  };

  template <class T>
    requires std::is_base_of<engine::App, T>::value
  inline int run(T& app) {
    auto time = std::chrono::high_resolution_clock::now();
    while (!app.shouldBail() && !app.shouldClose()) {
      auto now = std::chrono::high_resolution_clock::now();
      std::chrono::duration<float, std::milli> millis = now - time;
      float delta = millis.count() / 1000;
      time = now;

      FrameInfo frameInfo{app.getFrameIndex(), delta};

      app.update(frameInfo);
      app.render(frameInfo);
      app.postRender();
    }

    return app.shouldBail() ? -1 : 0;
  }
} // namespace engine