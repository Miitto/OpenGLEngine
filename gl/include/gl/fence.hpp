#pragma once

#include <gl/id.hpp>

namespace gl {
  class Fence {
    GLsync fence;

  public:
    Fence() { fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0); }

    ~Fence() {
      if (fence != nullptr) {
        glDeleteSync(fence);
      }
    }

    Fence(const Fence&) = delete;
    Fence& operator=(const Fence&) = delete;
    Fence(Fence&& other) noexcept {
      if (fence != nullptr) {
        glDeleteSync(fence);
      }
      fence = other.fence;
      other.fence = nullptr;
    }
    Fence& operator=(Fence&& other) noexcept {
      if (this != &other) {
        if (fence != nullptr) {
          glDeleteSync(fence);
        }

        fence = other.fence;
        other.fence = nullptr;
      }
      return *this;
    }

    GLsync get() const { return fence; }
    operator GLsync() const { return fence; }

    bool wait(GLuint64 timeout = GL_TIMEOUT_IGNORED) const {
      auto s = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, timeout);
      return s == GL_ALREADY_SIGNALED || s == GL_CONDITION_SATISFIED;
    }

    bool isValid() const { return fence != nullptr; }

    bool signalled() const {
      GLint value = 0;
      glGetSynciv(fence, GL_SYNC_STATUS, sizeof(GLint), nullptr, &value);
      return value == GL_SIGNALED;
    }

    void reset() {
      if (fence != nullptr) {
        glDeleteSync(fence);
      }
      fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }
  };
} // namespace gl