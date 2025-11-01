#include "engine/mesh/mesh.hpp"
#include "engine/mesh/mesh_data.hpp"
#include "logger.hpp"
#include <fstream>

using std::string;

namespace engine {
  Mesh::Mesh(const engine::mesh::Data& meshData, const BufferLocation& buffer,
             bool write)
      : startOffset(buffer.offset), vertexCount(meshData.vertices().size()),
        indexCount(meshData.indices().size()),
        meshLayers(meshData.meshLayers()), layerNames(meshData.layerNames()),
        indexOffset(findIndexOffset(meshData) + startOffset) {
    if (write) {
      auto stride = calculateVertexStride(meshData);
      BufferData(meshData, buffer, stride);
    }
  }

  GLuint Mesh::requiredSize(const mesh::Data& meshData) {
    auto indices =
        static_cast<GLuint>(meshData.indices().size() * sizeof(uint32_t));

    return findIndexOffset(meshData) + indices;
  }

  GLuint Mesh::findIndexOffset(const mesh::Data& bufferData) {
    auto verticesSize = bufferData.vertices().size() * sizeof(glm::vec3);
    auto colorsSize = bufferData.colors().size() * sizeof(glm::vec4);
    auto textureCoordsSize =
        bufferData.textureCoords().size() * sizeof(glm::vec2);
    auto normalsSize = bufferData.normals().size() * sizeof(glm::vec3);
    auto tangentsSize = bufferData.tangents().size() * sizeof(glm::vec4);
    return static_cast<GLuint>(verticesSize + colorsSize + textureCoordsSize +
                               normalsSize + tangentsSize);
  }

  GLuint Mesh::calculateVertexStride(const mesh::Data& bufferData) {
    GLuint stride = sizeof(glm::vec3); // positions are always present
    if (!bufferData.colors().empty()) {
      stride += sizeof(glm::vec4);
    }
    if (!bufferData.textureCoords().empty()) {
      stride += sizeof(glm::vec2);
    }
    if (!bufferData.normals().empty()) {
      stride += sizeof(glm::vec3);
    }
    if (!bufferData.tangents().empty()) {
      stride += sizeof(glm::vec4);
    }
    return stride;
  }

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

  void Mesh::BatchSubmeshes() {
    if (meshLayers.size() == 0) {
      return;
    }

    std::vector<GLsizei> counts;
    std::vector<GLvoid*> offsets;
    for (const auto& subMesh : meshLayers) {
      counts.push_back(static_cast<GLsizei>(subMesh.count));
      if (indexOffset != 0) {
        // Offset into the indices
        auto vertexOffset = subMesh.start * sizeof(unsigned int);
        // Offset into the entire buffer
        auto offset = indexOffset + vertexOffset;
        offsets.push_back(reinterpret_cast<GLvoid*>(offset));
      } else {
        offsets.push_back(reinterpret_cast<GLvoid*>(subMesh.start));
      }
    }

    vao.bind();
    if (indexOffset != 0) {
      glMultiDrawElements(
          type, counts.data(), GL_UNSIGNED_INT,
          reinterpret_cast<const GLvoid* const*>(offsets.data()),
          static_cast<GLsizei>(meshLayers.size()));
    } else {
      glMultiDrawArrays(type, reinterpret_cast<const GLint*>(offsets.data()),
                        counts.data(), static_cast<GLsizei>(meshLayers.size()));
    }
  }

  void Mesh::BufferData(const mesh::Data& bufferData,
                        const BufferLocation& buffer, GLuint stride) {
    auto& vertices = bufferData.vertices();
    auto& colors = bufferData.colors();
    auto& textureCoords = bufferData.textureCoords();
    auto& normals = bufferData.normals();
    auto& tangents = bufferData.tangents();
    auto& indices = bufferData.indices();

    auto vertexNum = vertices.size();
    if (colors.size() > vertexNum) {
      engine::Logger::warn(
          "Mesh::BufferData: colors size greater than vertices size!");
    }
    if (textureCoords.size() > vertexNum) {
      engine::Logger::warn(
          "Mesh::BufferData: textureCoords size greater than vertices size!");
    }
    if (normals.size() > vertexNum) {
      engine::Logger::warn(
          "Mesh::BufferData: normals size greater than vertices size!");
    }
    if (tangents.size() > vertexNum) {
      engine::Logger::warn(
          "Mesh::BufferData: tangents size greater than vertices size!");
    }

#define SZ(VEC, T) auto VEC##Size = static_cast<GLuint>(VEC.size() * sizeof(T))
    SZ(vertices, glm::vec3);
    SZ(colors, glm::vec4);
    SZ(textureCoords, glm::vec2);
    SZ(normals, glm::vec3);
    SZ(tangents, glm::vec4);
    SZ(indices, unsigned int);
#undef SZ

    vao.attribFormat(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    GLuint vPos = sizeof(glm::vec3);

#define AC(FIELD) !FIELD.empty()

    if (AC(colors)) {
      vao.attribFormat(1, 4, GL_FLOAT, GL_FALSE, vPos, 0);
      vPos += sizeof(glm::vec4);
    }
    if (AC(textureCoords)) {
      vao.attribFormat(2, 2, GL_FLOAT, GL_FALSE, vPos, 0);
      vPos += sizeof(glm::vec2);
    }
    if (AC(normals)) {
      vao.attribFormat(3, 3, GL_FLOAT, GL_FALSE, vPos, 0);
      vPos += sizeof(glm::vec3);
    }
    if (AC(tangents)) {
      vao.attribFormat(4, 4, GL_FLOAT, GL_FALSE, vPos, 0);
      vPos += sizeof(glm::vec4);
    }
    if (AC(indices)) {
      vao.bindIndexBuffer(buffer.id);
    }

    vao.bindVertexBuffer(0, buffer.id, startOffset, stride);

    auto& mapping = buffer.mapping;

#define WRITE(VEC, T)                                                          \
  if (AC(VEC)) {                                                               \
    mapping.write(&VEC[i], sizeof(T), offset);                                 \
    offset += sizeof(T);                                                       \
  }

    GLuint vStart = startOffset;
    for (size_t i = 0; i < vertices.size(); ++i) {
      auto offset = vStart;
      WRITE(vertices, glm::vec3);
      WRITE(colors, glm::vec4);
      WRITE(textureCoords, glm::vec2);
      WRITE(normals, glm::vec3);
      WRITE(tangents, glm::vec4);

      vStart += stride;
    }
#undef WRITE
#undef AC

    if (!indices.empty()) {
      mapping.write(indices.data(), indicesSize, indexOffset);
    }
  }

  bool Mesh::GetSubMesh(int i, const mesh::SubMesh* s) const {
    if (i < 0 || i >= (int)meshLayers.size()) {
      return false;
    }
    s = &meshLayers[i];
    return true;
  }

  bool Mesh::GetSubMesh(const string& name, const mesh::SubMesh* s) const {
    for (unsigned int i = 0; i < layerNames.size(); ++i) {
      if (layerNames[i] == name) {
        return GetSubMesh(i, s);
      }
    }
    return false;
  }
} // namespace engine