#pragma once

#include <concepts>
#include <engine/camera.hpp>
#include <utility>

namespace engine {
  template <typename L, typename R> class SplitCamera {
  public:
    SplitCamera(L&& left, R&& right, glm::ivec2 res)
        : leftCamera(std::move(left)), rightCamera(std::move(right)),
          windowSize(res) {}

    virtual ~SplitCamera() = default;

    SplitCamera() = delete;
    SplitCamera(const SplitCamera&) = delete;
    SplitCamera& operator=(const SplitCamera&) = delete;

    inline L& left() { return leftCamera; }
    inline R& right() { return rightCamera; }
    inline const L& left() const { return leftCamera; }
    inline const R& right() const { return rightCamera; }

    inline void leftView() {
      glViewport(0, 0, static_cast<int>(windowSize.x * (1.0f - splitRatio)),
                 windowSize.y);
    }

    inline void leftScissor() {
      glScissor(0, 0, static_cast<int>(windowSize.x * (1.0f - splitRatio)),
                windowSize.y);
    }

    inline void rightView() {
      glViewport(static_cast<int>(windowSize.x * (1.0f - splitRatio)), 0,
                 static_cast<int>(windowSize.x * splitRatio), windowSize.y);
    }

    inline void rightScissor() {
      glScissor(static_cast<int>(windowSize.x * (1.0f - splitRatio)), 0,
                static_cast<int>(windowSize.x * splitRatio), windowSize.y);
    }

    inline void fullView() { glViewport(0, 0, windowSize.x, windowSize.y); }

    inline void fullScissor() { glScissor(0, 0, windowSize.x, windowSize.y); }

    inline const float& getSplitRatio() const { return splitRatio; }
    inline void setSplitRatio(float ratio) {
      splitRatio = ratio;
      leftCamera.onResize(static_cast<int>(windowSize.x * (1.0f - splitRatio)),
                          windowSize.y, {0.0, 1.0f - splitRatio});
      rightCamera.onResize(static_cast<int>(windowSize.x * splitRatio),
                           windowSize.y, {1.0f - splitRatio, 1.0f});
      if (ratio == 0.0f) {
        leftActive = true;
      } else if (ratio == 1.0f) {
        leftActive = false;
      }
    }

    virtual void onResize(int width, int height) {
      windowSize = glm::ivec2(width, height);
      leftCamera.onResize(static_cast<int>(width * (1.0f - splitRatio)), height,
                          {0.0, 1.0f - splitRatio});
      rightCamera.onResize(static_cast<int>(width * splitRatio), height,
                           {1.0 - splitRatio, 1.0f});
    }

    virtual void update(const Input& input, float dt, bool acceptInput = true) {
      if (input.isKeyPressed(GLFW_KEY_TAB)) {
        leftActive = !leftActive;
      }

      if (leftActive) {
        leftCamera.update(input, dt, acceptInput);
      } else {
        rightCamera.update(input, dt, acceptInput);
      }
    }

  protected:
    L leftCamera;
    R rightCamera;
    float splitRatio = 0.0f; // Ratio of left to right view
    glm::ivec2 windowSize = glm::ivec2(800, 600);
    bool leftActive = true;
  };
} // namespace engine
