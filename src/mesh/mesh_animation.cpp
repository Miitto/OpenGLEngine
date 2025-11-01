#include "engine/mesh/mesh_animation.hpp"

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
    if (!file.is_open()) {
      Logger::error("Could not open mesh animation file: {}", filename);
      return;
    }

    std::string filetype;
    int fileVersion;

    file >> filetype;

    if (filetype != "MeshAnim") {
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
        for (int x = 0; x < 4; ++x) {
          for (int y = 0; y < 4; ++y) {
            file >> mat[x][y];
          }
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

    const glm::mat4* dataStart = allJoints.data() + (frame * jointCount);
    const std::span<const glm::mat4> dataSpan(dataStart, jointCount);

    return dataSpan;
  }
} // namespace engine::mesh