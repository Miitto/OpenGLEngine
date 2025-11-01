#pragma once

#include <engine/window.hpp>
#include <glm/glm.hpp>
#include <spdlog/fmt/bundled/format.h>
#include <unordered_map>

namespace engine {
  /// <summary>
  /// Holds mouse input data.
  /// </summary>
  struct Mouse {
    glm::vec2 position{};
    glm::vec2 delta{};
    int buttons = 0;

    glm::vec2 lastPosition() const { return position - delta; }
    void onClick(int button) { buttons |= (1 << button); }
    void onRelease(int button) { buttons &= ~(1 << button); }
    bool isButtonDown(int button) const {
      return (buttons & (1 << button)) != 0;
    }
  };

  /// <summary>
  /// Holds the state of a key.
  /// </summary>
  enum class KeyState {
    /// <summary>
    /// Key was pressed this frame.
    /// </summary>
    Down,
    /// <summary>
    /// Key was held down this frame.
    /// </summary>
    Held,
    /// <summary>
    /// Key was released this frame
    /// </summary>
    Up
  };

  /// <summary>
  /// Holds keyboard and mouse input data.
  /// </summary>
  class Input {
    engine::Window& m_window;
    Mouse m_mouse{};
    /// <summary>
    /// Key states. Absence from the map means Key is released, and was not
    /// released this frame.
    /// </summary>
    std::unordered_map<int, KeyState> keyState{};

    bool m_imguiWantsKeyboard = false;
    bool m_imguiWantsMouse = false;

  public:
    Input() = delete;
    Input(engine::Window& window) noexcept : m_window(window) {
      setupWindowCallbacks();
    }
    ~Input() = default;

    // No copy, due to window user pointer
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    Input(Input&& other) noexcept : m_window(other.m_window) {
      // If input is moved, we need to update the window user pointer to the new
      // location
      m_window.setUserPtr(this);
    }

    void onKeyEvent(int key, int action);
    void onMouseMove(double x, double y);
    void onMouseButton(int button, int action);

    void imGuiWantsKeyboard(bool wants) { m_imguiWantsKeyboard = wants; }
    void imGuiWantsMouse(bool wants) { m_imguiWantsMouse = wants; }

    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode,
                                int action, int mods);
    static void glfwCursorPosCallback(GLFWwindow* window, double xpos,
                                      double ypos);
    static void glfwMouseButtonCallback(GLFWwindow* window, int button,
                                        int action, int mods);

    void setupWindowCallbacks() {
      m_window.setUserPtr(this);
      m_window.setKeyCallback(Input::glfwKeyCallback);
      m_window.setCursorPosCallback(Input::glfwCursorPosCallback);
      m_window.setMouseButtonCallback(Input::glfwMouseButtonCallback);
    }

    KeyState getKeyState(int key) const {
      auto it = keyState.find(key);
      return it == keyState.end() ? KeyState::Up : it->second;
    }
    /// <summary>
    /// Returns if the key is currently pressed
    /// </summary>
    /// <param name="key"></param>
    /// <returns></returns>
    bool isKeyDown(int key) const {
      auto it = keyState.find(key);
      return it != keyState.end() &&
             (it->second == KeyState::Down || it->second == KeyState::Held);
    }
    /// <summary>
    /// Returns if the key is currently not pressed
    /// </summary>
    /// <param name="key"></param>
    /// <returns></returns>
    bool isKeyUp(int key) const {
      auto it = keyState.find(key);
      return it == keyState.end() || it->second == KeyState::Up;
    }
    /// <summary>
    /// Returns if the key was pressed this frame
    /// </summary>
    /// <param name="key"></param>
    /// <returns></returns>
    bool isKeyPressed(int key) const {
      auto it = keyState.find(key);
      return it != keyState.end() && it->second == KeyState::Down;
    }
    /// <summary>
    /// Returns if the key was released this frame
    /// </summary>
    /// <param name="key"></param>
    /// <returns></returns>
    bool isKeyReleased(int key) const {
      auto it = keyState.find(key);
      return it != keyState.end() && it->second == KeyState::Up;
    }

    /// <summary>
    /// Should be called as the frame is finishing. Clears per-frame input data
    /// and prepares for the next.
    /// </summary>
    void frameEnd();

    const Mouse& mouse() const { return m_mouse; }
  };
} // namespace engine
template <>
struct fmt::formatter<engine::KeyState> : fmt::formatter<std::string_view> {
  // parse is inherited from formatter<string_view>.

  auto format(engine::KeyState c, fmt::format_context& ctx) const
      -> fmt::format_context::iterator;
};
