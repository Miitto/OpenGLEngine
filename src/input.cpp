#include "engine/input.hpp"
#include "logger.hpp"

namespace engine {
  void Input::glfwKeyCallback(GLFWwindow* window, int key, int scancode,
                              int action, int mods) {
    Input* input = reinterpret_cast<Input*>(engine::Window::getUserPtr(window));
    input->onKeyEvent(key, action);

    (void)scancode;
    (void)mods;
  }

  void Input::glfwCursorPosCallback(GLFWwindow* window, double xpos,
                                    double ypos) {
    Input* input = reinterpret_cast<Input*>(engine::Window::getUserPtr(window));
    input->onMouseMove(xpos, ypos);
  }

  void Input::glfwMouseButtonCallback(GLFWwindow* window, int button,
                                      int action, int mods) {
    Input* input = reinterpret_cast<Input*>(engine::Window::getUserPtr(window));
    (void)mods;
    input->onMouseButton(button, action);
  }

  void Input::onKeyEvent(int key, int action) {
    if (m_imguiWantsKeyboard && action != GLFW_RELEASE) {
      return;
    }
    KeyState state = action == GLFW_PRESS    ? KeyState::Down
                     : action == GLFW_REPEAT ? KeyState::Held
                                             : KeyState::Up;
    keyState[key] = state;
    Logger::debug("Key {} is now {}", key, state);
  }
  void Input::onMouseMove(double x, double y) {
    if (m_imguiWantsMouse) {
      // If ImGui wants the mouse, don't update delta (still want position for
      // accurate tracking)
      m_mouse.position = {x, y};
      return;
    }
    m_mouse.delta += glm::vec2{x - m_mouse.position.x, y - m_mouse.position.y};
    m_mouse.position = {x, y};
  }

  void Input::onMouseButton(int button, int action) {
    if (m_imguiWantsMouse && action != GLFW_RELEASE) {
      return;
    }

    Logger::debug("Clicked at {}, {}", m_mouse.position.x, m_mouse.position.y);

    switch (action) {
    case GLFW_PRESS:
      m_mouse.onClick(button);
      break;
    case GLFW_RELEASE:
      m_mouse.onRelease(button);
      break;
    }
  }

  void Input::frameEnd() {
    m_mouse.delta = {0, 0};
    std::vector<int> keys;
    keys.reserve(keyState.size());
    for (auto [key, state] : keyState) {
      if (state == KeyState::Down) {
        state = KeyState::Held;
      } else if (state == KeyState::Up) {
        keys.push_back(key);
      }
    }

    for (auto& key : keys) {
      keyState.erase(key);
    }
  }

} // namespace engine
auto fmt::formatter<engine::KeyState>::format(engine::KeyState ks,
                                              fmt::format_context& ctx) const
    -> fmt::format_context::iterator {
  using engine::KeyState;

  std::string_view str = "";
  switch (ks) {
  case KeyState::Down:
    str = "Down";
    break;
  case KeyState::Held:
    str = "Held";
    break;
  case KeyState::Up:
    str = "Up";
    break;
  }
  return fmt::formatter<std::string_view>::format(str, ctx);
}
