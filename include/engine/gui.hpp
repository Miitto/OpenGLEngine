#pragma once

#include "engine/window.hpp"
#include <functional>
#include <imgui/imgui.h>

struct ImGuiIO;

namespace engine::gui {
  class Context {
    ImGuiIO& _io;

  public:
    Context(engine::Window& window);
    ~Context();

    void newFrame();
    void endFrame();

    ImGuiIO& io() { return _io; }

    void sleep(int ms);
  };

  class GuiWindow {
  public:
    GuiWindow(const char* name);
    ~GuiWindow();
  };

} // namespace engine::gui