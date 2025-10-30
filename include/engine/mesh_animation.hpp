#pragma once
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <span>

namespace engine::mesh {
  class Animation {
  public:
    Animation();
    Animation(const std::string& filename);
    ~Animation();

    unsigned int GetJointCount() const { return jointCount; }

    unsigned int GetFrameCount() const { return frameCount; }

    float GetFrameRate() const { return frameRate; }

    const std::span<const glm::mat4> GetJointData(unsigned int frame) const;

  protected:
    unsigned int jointCount;
    unsigned int frameCount;
    float frameRate;

    std::vector<glm::mat4> allJoints;
  };
} // namespace engine::mesh
