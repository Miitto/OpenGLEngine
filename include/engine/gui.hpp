#pragma once

#include "engine/window.hpp"
#include <functional>
#include <imgui/imgui.h>

struct ImGuiIO;

namespace engine::gui {
  /// <summary>
  /// RAII Wrapper for an ImGui context.
  /// </summary>
  class Context {
    ImGuiIO& _io;

  public:
    Context(engine::Window& window);
    ~Context();

    /// <summary>
    /// Should be called at the start of each frame.
    /// </summary>
    void newFrame();
    /// <summary>
    ///  Should be called at the end of each frame.
    /// </summary>
    void endFrame();

    ImGuiIO& io() { return _io; }

    /// <summary>
    /// Sleep the ui for the given time
    /// </summary>
    /// <param name="ms">Time in milliseconds</param>
    void sleep(int ms);
  };

  class GuiWindow {
  public:
    GuiWindow(const char* name);
    ~GuiWindow();
  };

} // namespace engine::gui