#pragma once

#include "engine/gui.hpp"
#include "engine/input.hpp"
#include "engine/scene_graph.hpp"
#include "engine/window.hpp"
#include "frame_info.hpp"
#include <chrono>

namespace engine {

  /// <summary>
  /// Base class for an application using this engine.
  /// Should be run with engine::run(app).
  /// </summary>
  class App {
  protected:
    engine::Window window;
    engine::Input input;
    engine::gui::Context gui;
    int flags = 0;
    uint32_t frameIndex = 0;

    /// <summary>
    /// Flags representing application options as a bitmask.
    /// </summary>
    enum AppFlags {
      NONE = 0,
      /// <summary>
      /// The application should exit as soon as possible due to an error.
      /// </summary>
      BAIL = 1 << 0,
    };

  public:
    App() = delete;
    /// <summary>
    /// Creates an application with the given window dimensions and title.
    /// Will initialize the window, input, and GUI context.
    /// </summary>
    /// <param name="width">Window width</param>
    /// <param name="height">Window height</param>
    /// <param name="title">Window title</param>
    App(int width, int height, const char title[]);

    /// <summary>
    /// Function called every frame to update application state.
    /// </summary>
    /// <param name="frame">Infomation about the current frame</param>
    virtual void update(const FrameInfo& frame);
    /// <summary>
    /// Function called every frame to render the application.
    /// </summary>
    /// <param name="frame">Infomation about the current frame</param>
    virtual void render(const FrameInfo& frame) = 0;

    /// <summary>
    /// Make the application exit as soon as possible.
    /// </summary>
    void bail() { flags |= BAIL; }
    /// <summary>
    /// Whether the application is about to exit due to an error.
    /// </summary>
    /// <returns></returns>
    bool shouldBail() const { return (flags & BAIL) != 0; }
    /// <summary>
    /// Whether the application window has been requested to close, for a
    /// graceful shutdown.
    /// </summary>
    /// <returns></returns>
    bool shouldClose() const { return window.shouldClose(); }

    /// <summary>
    /// Index of the current frame.
    /// The 4th frame will have index 3.
    /// </summary>
    /// <returns>Frame index</returns>
    uint32_t getFrameIndex() const { return frameIndex; }

    /// <summary>
    /// Called after rendering each frame.
    /// Handles post-render tasks such as rendering the UI, swapping buffers,
    /// and clearing input states.
    /// </summary>
    void postRender() {
      input.frameEnd();
      gui.endFrame();
      window.swapBuffers();

      ++frameIndex;
    }
  };

  /// <summary>
  /// Runs the given application.
  /// </summary>
  /// <typeparam name="T">User defined class deriving from
  /// engine::App</typeparam> <param name="app">The app to run</param>
  /// <returns>Exit status that main should return</returns>
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