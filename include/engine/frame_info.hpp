#pragma once

namespace engine {
  /// <summary>
  /// Data about the current frame.
  /// </summary>
  struct FrameInfo {
    uint32_t frameIndex;
    float frameDelta;
  };
} // namespace engine