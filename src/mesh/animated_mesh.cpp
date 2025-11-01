#include "engine/mesh/animated_mesh.hpp"

#include "engine/mesh/mesh_data.hpp"
#include "logger.hpp"
#include <fstream>
#include <gl/attribs.hpp>

using std::string;

namespace {
  constexpr GLuint MAX_JOINTS = 128;
}

namespace engine {
  AnimatedMesh::AnimatedMesh(const mesh::Data& meshData,
                             Mesh::BufferLocation buffer,
                             const mesh::Animation& animation)
      : Mesh(meshData, buffer, false),
        jointSize(calculateJointDataSize(animation)) {
#ifndef NDEBUG
    if (animation.GetJointCount() > MAX_JOINTS) {
      engine::Logger::warn(
          "AnimatedMesh::AnimatedMesh: joint count {} exceeds maximum of {}",
          animation.GetJointCount(), MAX_JOINTS);
    }
#endif

    // The base Mesh constructor does not account for all our vertex attributes
    indexOffset = startOffset + findIndexOffset(meshData);

    auto stride = calculateVertexStride(meshData);

    auto indexSize =
        static_cast<GLuint>(meshData.indices().size() * sizeof(uint32_t));
    auto indexEnd = indexOffset + indexSize;

    // Round indexEnd up to gl::UNIFORM_BUFFER_OFFSET_ALIGNMENT
    auto remainder = indexEnd % gl::UNIFORM_BUFFER_OFFSET_ALIGNMENT;
    if (remainder != 0) {
      indexEnd += gl::UNIFORM_BUFFER_OFFSET_ALIGNMENT - remainder;
    }
    jointOffset = indexEnd;

    BufferData(meshData, buffer, stride, animation);
  }

  GLuint AnimatedMesh::requiredSize(const mesh::Data& meshData,
                                    const mesh::Animation& animation) {
    auto indexOffset = findIndexOffset(meshData);
    auto indexSize =
        static_cast<GLuint>(meshData.indices().size() * sizeof(uint32_t));
    auto indexEnd = indexOffset + indexSize;

    auto jointOffset = indexEnd;
    // Round up to gl::UNIFORM_BUFFER_OFFSET_ALIGNMENT
    auto remainder = jointOffset % gl::UNIFORM_BUFFER_OFFSET_ALIGNMENT;
    if (remainder != 0) {
      jointOffset += gl::UNIFORM_BUFFER_OFFSET_ALIGNMENT - remainder;
    }
    auto jointSize = calculateJointDataSize(animation);
    return jointOffset + jointSize;
  }

  GLuint AnimatedMesh::findIndexOffset(const mesh::Data& meshData) {
    auto base = Mesh::findIndexOffset(meshData);

    auto extraVertAttrsSize = 0;
    extraVertAttrsSize +=
        static_cast<GLuint>(meshData.weights().size() * sizeof(glm::vec4));
    extraVertAttrsSize += static_cast<GLuint>(meshData.weightIndices().size() *
                                              sizeof(glm::ivec4));
    return base + extraVertAttrsSize;
  }

  GLuint AnimatedMesh::calculateVertexStride(const mesh::Data& meshData) {
    auto baseStride = Mesh::calculateVertexStride(meshData);
    if (!meshData.weights().empty()) {
      baseStride += sizeof(glm::vec4);
    }
    if (!meshData.weightIndices().empty()) {
      baseStride += sizeof(glm::ivec4);
    }
    return baseStride;
  }

  GLuint
  AnimatedMesh::calculateJointDataSize(const mesh::Animation& animation) {
    return animation.GetFrameCount() * animation.GetJointCount() *
           sizeof(glm::mat4);
  }

  void AnimatedMesh::BufferData(const mesh::Data& meshData,
                                const Mesh::BufferLocation& buffer,
                                GLuint stride,
                                const engine::mesh::Animation& animation) {
    Mesh::BufferData(meshData, buffer, stride);
    const auto meshStride = Mesh::calculateVertexStride(meshData);

    auto& vertices = meshData.vertices();
    auto& weights = meshData.weights();
    auto& weightIndices = meshData.weightIndices();

    auto vertexNum = vertices.size();
#ifndef NDEBUG
    if (weights.size() > vertexNum) {
      engine::Logger::warn(
          "Mesh::BufferData: weights size greater than vertices size!");
    }
    if (weightIndices.size() > vertexNum) {
      engine::Logger::warn(
          "Mesh::BufferData: weightIndices size greater than vertices size!");
    }
#endif

#define AC(FIELD) !FIELD.empty()
    auto vPos = meshStride;
    if (AC(weights)) {
      vao.attribFormat(5, 4, GL_FLOAT, GL_FALSE, vPos, 0);
      vPos += sizeof(glm::vec4);
    }
    if (AC(weightIndices)) {
      vao.attribFormat(6, 4, GL_INT, GL_FALSE, vPos, 0);
      vPos += sizeof(glm::ivec4);
    }

    auto& mapping = buffer.mapping;
#define WRITE(VEC, T)                                                          \
  if (AC(VEC)) {                                                               \
    mapping.write(&VEC[i], sizeof(T), offset);                                 \
    offset += sizeof(T);                                                       \
  }

    GLuint vStart = startOffset + meshStride;
    for (size_t i = 0; i < vertices.size(); ++i) {
      auto offset = vStart;
      WRITE(weights, glm::vec4);
      WRITE(weightIndices, glm::ivec4);

      vStart += stride;
    }
#undef WRITE
#undef AC

    const auto& invBindPose = meshData.inverseBindPose();
    const auto jointCount = animation.GetJointCount();
    const auto frameCount = animation.GetFrameCount();

    auto offset = jointOffset;
    for (size_t frame = 0; frame < animation.GetFrameCount(); ++frame) {
      auto jointData = animation.GetJointData(frame);

      for (size_t j = 0; j < animation.GetJointCount(); ++j) {
        glm::mat4 anim = jointData[j] * invBindPose[j];
        mapping.write(&anim, sizeof(glm::mat4), offset);
        offset += sizeof(glm::mat4);
      }
    }
    auto jointEnd = jointOffset + jointSize;
    if (offset != jointEnd) {
      engine::Logger::warn(
          "AnimatedMesh::BufferData: joint data size mismatch (expected {}, "
          "got {})",
          jointSize, offset - jointOffset);
    }
  }
} // namespace engine
