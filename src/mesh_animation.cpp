#include "engine/mesh_animation.hpp"

#include "logger.hpp"
#include <fstream>
#include <glm/glm.hpp>
#include <string>

namespace engine::mesh {
  Animation::Animation() {
    jointCount = 0;
    frameCount = 0;
    frameRate = 0.0f;
  }

  Animation::Animation(const std::string& filename) : Animation() {
    std::ifstream file(filename);

    std::string filetype;
    int fileVersion;

    file >> filetype;

    if (filetype != "Anim") {
      engine::Logger::error(
          "Loading mesh animation from file: {}. Not a  Anim file", filename);
      return;
    }
    file >> fileVersion;
    file >> frameCount;
    file >> jointCount;
    file >> frameRate;

    allJoints.reserve(frameCount * jointCount);

    for (unsigned int f = 0; f < frameCount; ++f) {
      for (unsigned int j = 0; j < jointCount; ++j) {
        glm::mat4 mat;
        for (int i = 0; i < 16; ++i) {
          file >> mat[i / 4][i % 4];
        }
        allJoints.emplace_back(mat);
      }
    }
  }

  Animation::~Animation() {}

  const std::span<const glm::mat4>
  Animation::GetJointData(unsigned int frame) const {
    if (frame >= frameCount) {
      throw std::runtime_error("Index out of range");
    }

    const glm::mat4* dataStart = allJoints.data() + frame;
    const std::span<const glm::mat4> dataSpan(dataStart, jointCount - frame);

    return dataSpan;
  }
} // namespace engine::mesh