#include <engine/mesh/basic.hpp>

namespace engine::mesh {

  BasicMesh::BasicMesh(const engine::mesh::Data& meshData) {
    vertexCount = meshData.vertices().size();
    indexCount = meshData.indices().size();

    GLuint vertexSize = static_cast<GLuint>(sizeof(glm::vec3) * vertexCount);
    GLuint indexSize = static_cast<GLuint>(sizeof(uint32_t) * indexCount);

    vertices.init(vertexSize, meshData.vertices().data());

    if (indexCount > 0) {
      indices.init(indexSize, meshData.indices().data());
      vao.bindIndexBuffer(indices.id());
    }

    vao.bindVertexBuffer(0, vertices.id(), 0, sizeof(glm::vec3));
    vao.attribFormat(0, 3, GL_FLOAT, false, 0, 0);
  }

  void BasicMesh::draw() const {
    if (indexCount > 0) {
      glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    } else {
      glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
  }

} // namespace engine::mesh