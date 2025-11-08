#include "engine/mesh/mesh.hpp"
#include "engine/mesh/mesh_data.hpp"
#include "logger.hpp"
#include <gl/structs.hpp>

namespace engine::mesh {
  Mesh::Mesh(const mesh::Data& meshData, std::vector<TextureSet>&& textureSets)
      : meshLayers(meshData.meshLayers()), layerNames(meshData.layerNames()),
        textureSets(std::move(textureSets)) {
#ifndef NDEBUG
    if (this->textureSets.size() != this->layerNames.size()) {
      engine::Logger::critical(
          "Mesh created with differing number of texture sets and layer "
          "names!");
      abort();
    }
#endif
  }

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

  void Mesh::writeTextureSets(gl::MappingRef& mapping) const {
    std::vector<engine::mesh::TextureHandleSet> textureHandles;
    textureHandles.reserve(textureSets.size());
    for (const auto& textureSet : textureSets) {
      textureHandles.push_back(textureSet.handles);
    }

    auto textureSize =
        static_cast<GLuint>(textureHandles.size() * sizeof(TextureHandleSet));
    mapping.write(textureHandles.data(), textureSize, 0);
    mapping += textureSize;
  }

  void Mesh::writeVertexData(const mesh::Data& meshData,
                             GLuint& vertexStartIndex,
                             const gl::MappingRef stagingMapping) {
    vertexOffset = vertexStartIndex;
    vertexCount = static_cast<uint32_t>(meshData.vertices().size());

    auto& vertices = meshData.vertices();
    auto& textureCoords = meshData.textureCoords();
    auto& normals = meshData.normals();
    auto& tangents = meshData.tangents();
    auto& weights = meshData.weights();
    auto& weightIndices = meshData.weightIndices();

    auto& indices = meshData.indices();

    auto vertexNum = vertices.size();

#ifndef NDEBUG
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

#define AC(FIELD) !FIELD.empty()

    {
      GLuint written = 0;
      GLuint vStart = 0;
      for (size_t i = 0; i < vertices.size(); ++i) {
        auto offset = vStart;

        WeightedVertex vertex{
            .position = vertices[i],
            .texCoord = AC(textureCoords) ? textureCoords[i] : glm::vec2(0.0f),
            .normal = AC(normals) ? normals[i] : glm::vec3(0.0f, 0.0f, 1.0f),
            .tangent =
                AC(tangents) ? tangents[i] : glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
            .jointWeights = AC(weights) ? weights[i] : glm::vec4(0.0f),
            .jointIndices =
                AC(weightIndices) ? weightIndices[i] : glm::ivec4(0),
        };

        stagingMapping.write(&vertex, sizeof(WeightedVertex), offset);

        vStart += sizeof(WeightedVertex);
        ++written;
      }
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