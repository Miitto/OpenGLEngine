#pragma once

struct GLFWwindow;

#include <functional>
#include <optional>
#include <string>

namespace engine {
  /// <summary>
  /// Window Manager that manages the GLFW state. Should be created before any
  /// windows and should persist to after the last one closes.
  /// </summary>
  class WindowManager {

    static bool initGlfw();

  public:
    WindowManager() = delete;

    static std::optional<std::string> initialize();
    static std::function<void()> shutdown();
  };

  /// <summary>
  /// An individual window. Requires a <c>WindowManager</c> to be created first.
  /// </summary>
  class Window {

  public:
    /// <summary>
    /// Constructs a Window object with the specified dimensions and title.
    /// </summary>
    /// <param name="width">The width of the window in pixels.</param>
    /// <param name="height">The height of the window in pixels.</param>
    /// <param name="title">The title of the window.</param>
    /// <param name="makeCurrent">Whether to make the window the current context
    /// upon creation. Defaults to false.</param>
    Window(int width, int height, const char* title, bool fullscreen = false,
           bool makeCurrent = false);
    ~Window();
    operator GLFWwindow*() const { return window; }

    void fullscreen(bool enable);
    bool isFullscreen() const { return fullscreened; }

    void makeCurrent() const;
    bool shouldClose() const;
    void swapBuffers() const;
    static void pollEvents();

    void setUserPtr(void* ptr);
    static void* getUserPtr(GLFWwindow* window);
    void setKeyCallback(GLFWkeyfun callback) {
      glfwSetKeyCallback(window, callback);
    }
    void setCursorPosCallback(GLFWcursorposfun callback) {
      glfwSetCursorPosCallback(window, callback);
    }
    void setMouseButtonCallback(GLFWmousebuttonfun callback) {
      glfwSetMouseButtonCallback(window, callback);
    }

    struct Size {
      int width;
      int height;

      friend bool operator==(const Size& a, const Size& b) {
        return a.width == b.width && a.height == b.height;
      }
    };

    Size size() const;

  protected:
    GLFWwindow* window;
    bool fullscreened;
    Size cachedSize;
  };
} // namespace engine
