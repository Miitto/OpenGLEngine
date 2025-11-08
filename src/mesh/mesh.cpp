#include "engine/mesh/mesh.hpp"
#include "engine/mesh/mesh_data.hpp"
#include "logger.hpp"
#include <gl/structs.hpp>

namespace engine::mesh {
  Mesh::Mesh(const mesh::Data& meshData)
      : meshLayers(meshData.meshLayers()), layerNames(meshData.layerNames()) {}

  GLuint Mesh::writeBatchedDraws(gl::MappingRef& mapping, GLuint baseVertex,
                                 GLuint instances, GLuint baseInstance) const {
    std::vector<gl::DrawElementsIndirectCommand> draws(meshLayers.size());
    for (size_t i = 0; i < meshLayers.size(); ++i) {
      const auto& layer = meshLayers[i];
      draws[i].count = layer.count;
      draws[i].instanceCount = instances;
      draws[i].firstIndex = layer.start + indexOffset;
      draws[i].baseVertex = baseVertex;
      draws[i].baseInstance = baseInstance;
    }
    auto size = static_cast<GLuint>(draws.size() *
                                    sizeof(gl::DrawElementsIndirectCommand));
    mapping.write(draws.data(), size, 0);
    mapping += size;
    return static_cast<GLuint>(draws.size());
  }

  GLuint Mesh::vertexDataSize(const mesh::Data& meshData) {
    auto vertexNum = static_cast<GLuint>(meshData.vertices().size());
    return vertexNum * vertexStride();
  }

  Mesh::AlignedSize Mesh::indexDataSize(const mesh::Data& meshData,
                                        GLuint startOffset) {
    auto offset = gl::Buffer::roundToAlignment(startOffset, sizeof(uint32_t));
    auto indexSize =
        static_cast<GLuint>(meshData.indices().size() * sizeof(uint32_t));
    auto totalSize = indexSize + (offset - startOffset);

    return {.offset = offset, .size = totalSize, .alignedSize = indexSize};
  }

  Mesh::AlignedSize Mesh::jointDataSize(const mesh::Animation& animation,
                                        GLuint startOffset) {
    auto offset = gl::Buffer::roundToAlignment(
        startOffset, gl::UNIFORM_BUFFER_OFFSET_ALIGNMENT);
    auto jointSize =
        static_cast<GLuint>(animation.GetFrameCount() *
                            animation.GetJointCount() * sizeof(glm::mat4));
    auto totalSize = jointSize + (offset - startOffset);
    return {.offset = offset, .size = totalSize, .alignedSize = jointSize};
  }

  GLuint Mesh::indirectBufferSize() const {
    return static_cast<GLuint>(meshLayers.size() *
                               sizeof(gl::DrawElementsIndirectCommand));
  }

  void Mesh::writeVertexData(const mesh::Data& meshData,
                             GLuint& vertexStartIndex,
                             const gl::MappingRef stagingMapping) {
    vertexOffset = vertexStartIndex;

    auto& vertices = meshData.vertices();
    auto& colors = meshData.colors();
    auto& textureCoords = meshData.textureCoords();
    auto& normals = meshData.normals();
    auto& tangents = meshData.tangents();
    auto& weights = meshData.weights();
    auto& weightIndices = meshData.weightIndices();

    auto& indices = meshData.indices();

    auto vertexNum = vertices.size();

#ifndef NDEBUG
    if (colors.size() > vertexNum) {
      engine::Logger::warn(
          "Mesh data: colors size greater than vertices size!");
    }
    if (textureCoords.size() > vertexNum) {
      engine::Logger::warn(
          "Mesh data: textureCoords size greater than vertices size!");
    }
    if (normals.size() > vertexNum) {
      engine::Logger::warn(
          "Mesh data: normals size greater than vertices size!");
    }
    if (tangents.size() > vertexNum) {
      engine::Logger::warn(
          "Mesh data: tangents size greater than vertices size!");
    }
    if (weights.size() > vertexNum) {
      engine::Logger::warn(
          "Mesh data: weights size greater than vertices size!");
    }
    if (weightIndices.size() > vertexNum) {
      engine::Logger::warn(
          "Mesh data: weightIndices size greater than vertices size!");
    }
#endif

    constexpr GLuint stride = Mesh::vertexStride();

#define AC(FIELD) !FIELD.empty()

    {
#define WRITE(VEC, T)                                                          \
  if (AC(VEC)) {                                                               \
    stagingMapping.write(&VEC[i], sizeof(T), offset);                          \
  }                                                                            \
  offset += sizeof(T);

      GLuint written = 0;
      GLuint vStart = 0;
      for (size_t i = 0; i < vertices.size(); ++i) {
        auto offset = vStart;
        WRITE(vertices, glm::vec3);
        WRITE(colors, glm::vec4);
        WRITE(textureCoords, glm::vec2);
        WRITE(normals, glm::vec3);
        WRITE(tangents, glm::vec4);
        WRITE(weights, glm::vec4);
        WRITE(weightIndices, glm::ivec4);

        vStart += stride;
        ++written;
      }
#undef WRITE
#undef AC

      vertexStartIndex += written;
    }
  }

  void Mesh::writeIndexData(const engine::mesh::Data& meshData,
                            GLuint& indexOffset,
                            const gl::MappingRef stagingMapping) {

#ifndef NDEBUG
    if (indexOffset % sizeof(uint32_t) != 0) {
      engine::Logger::warn(
          "Mesh data: index buffer offset is not aligned to uint32_t!");
    }
#endif

    this->indexOffset = indexOffset / sizeof(uint32_t);

    auto& indices = meshData.indices();
    auto size = static_cast<GLuint>(indices.size() * sizeof(uint32_t));
    stagingMapping.write(indices.data(), size, 0);
    indexOffset += size;
  }

  void Mesh::writeJointData(const mesh::Data& meshData,
                            const mesh::Animation& animation,
                            const gl::MappingRef stagingMapping,
                            uint32_t startJointIndex) {
    auto offset = stagingMapping.getOffset();
#ifndef NDEBUG
    if (offset % gl::UNIFORM_BUFFER_OFFSET_ALIGNMENT != 0) {
      engine::Logger::warn("Mesh data: joint buffer offset is not aligned to "
                           "GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT!");
    }
#endif

    auto& invBindPose = meshData.inverseBindPose();
    auto jointCount = animation.GetJointCount();

    std::vector<glm::mat4> jointMatrices(jointCount);

    for (int frame = 0; frame < animation.GetFrameCount(); ++frame) {
      auto joints = animation.GetJointData(frame);

      for (size_t j = 0; j < jointCount; ++j) {
        jointMatrices[j] = joints[j] * invBindPose[j];
      }
      auto offset = frame * jointCount * sizeof(glm::mat4);
      stagingMapping.write(jointMatrices.data(),
                           jointMatrices.size() * sizeof(glm::mat4), offset);
    }

    this->startJointIndex = startJointIndex;
    this->frameCount = animation.GetFrameCount();
    this->jointCount = jointCount;
    this->oneOverFrameRate = 1.0f / animation.GetFrameRate();
  }

  bool Mesh::GetSubMesh(int i, const mesh::SubMesh* s) const {
    if (i < 0 || i >= (int)meshLayers.size()) {
      return false;
    }
    s = &meshLayers[i];
    return true;
  }

  bool Mesh::GetSubMesh(const std::string& name, const mesh::SubMesh* s) const {
    for (unsigned int i = 0; i < layerNames.size(); ++i) {
      if (layerNames[i] == name) {
        return GetSubMesh(i, s);
      }
    }
    return false;
  }
} // namespace engine::mesh