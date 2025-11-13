#pragma once

#include <engine/mesh/mesh_data.hpp>
#include <gl/gl.hpp>

namespace engine::mesh {
  class BasicMesh {
  public:
    BasicMesh() = default;
    BasicMesh(const engine::mesh::Data& meshData);

    void bind() const { vao.bind(); }
    void unbind() const { vao.unbind(); }
    gl::Vao::BindGuard bindGuard() const { return vao.bindGuard(); }
    void draw() const;

  protected:
    gl::Buffer vertices = {};
    gl::Buffer indices = {};
    GLuint vertexCount = 0;
    GLuint indexCount = 0;
    gl::Vao vao = {};
  };
} // namespace engine::mesh