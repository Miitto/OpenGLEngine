#pragma once

#include "engine/frame_info.hpp"
#include "engine/gui.hpp"
#include "engine/input.hpp"
#include "engine/scene_graph.hpp"
#include "engine/window.hpp"
#include <chrono>

namespace engine {
  /// <summary>
  /// Base class for an application using this engine.
  /// Should be run with engine::run(app).
  /// </summary>
  class App {
  protected:
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
    ~App();

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

    virtual void onWindowResize(engine::Window::Size newSize);

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

    template <typename T> static std::optional<std::string> registerPlugin() {
      std::optional<std::string> result = T::initialize();
      if (result.has_value()) {
        return result;
      }

      engine::App::pluginShutdowns.push_back(T::shutdown());
      return std::nullopt;
    }

  protected:
    static bool initialized;
    engine::Window window;
    engine::Input input;
    engine::gui::Context gui;
    int flags = 0;
    uint32_t frameIndex = 0;
    engine::Window::Size windowSize;

  public:
    struct GBuffers {
      gl::Texture diffuse;
      gl::Texture normal;
      gl::Texture material;
      gl::Texture depth;
      gl::Texture stencil;
      gl::Framebuffer fbo;
    };

  protected:
    GBuffers gbuffers;

    static std::vector<std::function<void()>> pluginShutdowns;
  };

  /// <summary>
  /// Loads the necessary engine plugins that must load BEFORE the app is
  /// created.
  /// </summary>
  /// <returns>Error message if a plugin errored</returns>
  std::optional<std::string> loadPreInitEnginePlugins();

  /// <summary>
  /// Loads the necessary engine plugins that must load AFTER the app is
  /// created.
  /// </summary>
  /// <returns>Error message if a plugin errored</returns>
  std::optional<std::string> loadPostInitEnginePlugins();

  namespace __private {
    void logStart();
  }

  /// <summary>
  /// Runs the given application.
  /// </summary>
  /// <typeparam name="T">User defined class deriving from
  /// engine::App</typeparam> <param name="app">The app to run</param>
  /// <returns>Exit status that main should return</returns>
  template <class T>
    requires std::is_base_of<engine::App, T>::value
  inline int run(T& app) {
    __private::logStart();
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