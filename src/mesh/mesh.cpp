#include "engine/mesh/mesh.hpp"
#include "engine/mesh/mesh_data.hpp"
#include "logger.hpp"
#include <gl/structs.hpp>

namespace engine {
  Mesh::Mesh(const mesh::Data& meshData)
      : meshLayers(meshData.meshLayers()), layerNames(meshData.layerNames()) {}

  void Mesh::Draw() {
    auto bg = vao.bindGuard();
    if (indexOffset != 0) {
      uintptr_t offset = static_cast<uintptr_t>(indexOffset);
      glDrawElements(type, indexCount, GL_UNSIGNED_INT,
                     reinterpret_cast<GLvoid*>(offset));
    } else {
      glDrawArrays(type, 0, vertexCount);
    }
  }

  void Mesh::DrawSubMesh(int i) {
    if (i < 0 || i >= (int)meshLayers.size()) {
      return;
    }
    mesh::SubMesh m = meshLayers[i];

    vao.bind();
    if (indexOffset != 0) {
      auto vertexOffset = m.start * sizeof(unsigned int);

      auto offset = indexOffset + vertexOffset;
      glDrawElements(type, m.count, GL_UNSIGNED_INT, (const GLvoid*)offset);
    } else {
      glDrawArrays(type, m.start, m.count); // Draw the triangle!
    }
    vao.unbind();
  }

  void Mesh::prepSubmeshesForBatch(const gl::Mapping& indirectBufferMapping,
                                   GLuint offset, uint32_t instances,
                                   uint32_t baseInstance) const {
    std::vector<gl::DrawElementsIndirectCommand> drawCommands;
    drawCommands.reserve(meshLayers.size());
    uint32_t indexOffsetAsIndexCount =
        static_cast<uint32_t>(indexOffset / sizeof(uint32_t));
    for (const auto& subMesh : meshLayers) {
      gl::DrawElementsIndirectCommand cmd = {
          .count = static_cast<uint32_t>(subMesh.count),
          .instanceCount = instances,
          .firstIndex =
              static_cast<uint32_t>(subMesh.start) + indexOffsetAsIndexCount,
          .baseVertex = 0,
          .baseInstance = baseInstance};
      drawCommands.push_back(cmd);
    }
    indirectBufferMapping.write(
        drawCommands.data(),
        drawCommands.size() * sizeof(gl::DrawElementsIndirectCommand), offset);
  }

  void Mesh::BatchSubmeshes(const gl::Buffer& buffer, GLuint offset) {
    if (meshLayers.size() == 0) {
      return;
    }

    vao.bind();
    if (jointBuffer != 0) {
      glBindBufferRange(static_cast<GLenum>(gl::Buffer::StorageTarget::STORAGE),
                        jointBindPoint, jointBuffer, jointOffset, jointSize);
    }
    buffer.bind(gl::Buffer::BasicTarget::DRAW_INDIRECT);

    uintptr_t offsetPtr = static_cast<uintptr_t>(offset);

    glMultiDrawElementsIndirect(type, GL_UNSIGNED_INT, (const void*)offsetPtr,
                                meshLayers.size(), 0);
  }

  void Mesh::setupInstanceAttr(GLuint index, GLuint size, GLenum type,
                               GLboolean normalize, GLuint offset,
                               GLuint bufferIndex) {
    vao.attribFormat(index, size, type, normalize, offset, bufferIndex);
  }

  void Mesh::bindInstanceBuffer(GLuint index, const gl::Buffer& buffer,
                                GLuint offset, GLuint stride, GLuint divisor) {
    vao.bindVertexBuffer(index, buffer.id(), offset, stride);
    vao.attribDivisor(index, divisor);
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
                             const BufferLocation& buffer,
                             const gl::MappingRef stagingMapping) {
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
    vao.attribFormat(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    GLuint vPos = sizeof(glm::vec3);

    vao.attribFormat(1, 4, GL_FLOAT, GL_FALSE, vPos, 0);
    vPos += sizeof(glm::vec4);
    vao.attribFormat(2, 2, GL_FLOAT, GL_FALSE, vPos, 0);
    vPos += sizeof(glm::vec2);
    vao.attribFormat(3, 3, GL_FLOAT, GL_FALSE, vPos, 0);
    vPos += sizeof(glm::vec3);
    vao.attribFormat(4, 4, GL_FLOAT, GL_FALSE, vPos, 0);
    vPos += sizeof(glm::vec4);
    vao.attribFormat(5, 4, GL_FLOAT, GL_FALSE, vPos, 0);
    vPos += sizeof(glm::vec4);
    vao.attribFormat(6, 4, GL_INT, GL_FALSE, vPos, 0);

    vao.bindVertexBuffer(0, buffer.buffer.id(), buffer.offset, stride);

    {
#define WRITE(VEC, T)                                                          \
  if (AC(VEC)) {                                                               \
    stagingMapping.write(&VEC[i], sizeof(T), offset);                          \
  }                                                                            \
  offset += sizeof(T);

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
      }
#undef WRITE
#undef AC
    }
  }

  void Mesh::writeIndexData(const engine::mesh::Data& meshData,
                            const BufferLocation& buffer,
                            const gl::MappingRef stagingMapping) {
#ifndef NDEBUG
    if (buffer.offset % sizeof(uint32_t) != 0) {
      engine::Logger::warn(
          "Mesh data: index buffer offset is not aligned to uint32_t!");
    }
#endif

    indexOffset = buffer.offset / sizeof(uint32_t);

    auto& indices = meshData.indices();
    stagingMapping.write(indices.data(), indices.size() * sizeof(uint32_t), 0);
    vao.bindIndexBuffer(buffer.buffer.id());
  }

  void Mesh::writeJointData(const mesh::Data& meshData,
                            const mesh::Animation& animation,
                            const BufferLocation& buffer, GLuint jointBindPoint,
                            const gl::MappingRef stagingMapping) {
#ifndef NDEBUG
    if (buffer.offset % gl::UNIFORM_BUFFER_OFFSET_ALIGNMENT != 0) {
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

    jointOffset = buffer.offset;
    jointSize = jointCount * sizeof(glm::mat4) * animation.GetFrameCount();
    this->jointBindPoint = jointBindPoint;
    jointBuffer = gl::Id(static_cast<GLuint>(buffer.buffer.id()));
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
} // namespace engine