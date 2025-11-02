#pragma once

#include <functional>
#include <optional>
#include <string>

namespace engine {
  class GlLoader {
    static bool loadedGl;
    static bool loadGl();

  public:
    inline static bool isGlLoaded() { return loadedGl; }
    static void glMajor(int major);
    static void glMinor(int minor);
    static void glVersion(int major, int minor) {
      glMajor(major);
      glMinor(minor);
    }

    inline static std::optional<std::string> initialize() {
      if (!loadGl()) {
        return std::string("Failed to load OpenGL");
      }
      return std::nullopt;
    }
    inline static std::function<void()> shutdown() {
      return []() {};
    }
  };
} // namespace engine