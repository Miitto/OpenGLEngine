/******************************************************************************
Class:Mesh
Implements:
Author:Rich Davison	 <richard-gordon.davison@newcastle.ac.uk>
Description:Wrapper around OpenGL primitives, geometry and related
OGL functions.

There's a couple of extra functions in here that you didn't get in the tutorial
series, to draw debug normals and tangents.


-_-_-_-_-_-_-_,------,
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "engine/mesh/mesh_data.hpp"
#include <array>
#include <gl/gl.hpp>
#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <vector>

namespace engine {

  // TODO: Mesh class that uses an externally owned buffer with an offset

  /// <summary>
  /// Utilities to do with meshes.
  /// This mesh owns one buffer that holds all mesh data.
  /// </summary>
  /// <remarks>
  /// Attribute Layout:
  /// 0. vec3 position
  /// 1. vec4 color
  /// 2. vec2 texture coordinate
  /// 3. vec3 normal
  /// 4. vec4 tangent
  /// </remarks>
  class Mesh {
  public:
    Mesh(void) = default;
    ~Mesh(void) = default;
    Mesh(const Mesh& other) = delete;
    Mesh& operator=(const Mesh& other) = delete;
    Mesh(Mesh&& other) = default;
    Mesh& operator=(Mesh&& other) = default;

    /// <summary>
    /// Draws the entire mesh. Expects any necessary objects to be bound as the
    /// mesh only binds its VAO
    /// </summary>
    void Draw();
    /// <summary>
    /// Draws a specific sub-mesh. Expects any necessary objects to be bound as
    /// the mesh only binds its VAO
    /// </summary>
    /// <param name="i"></param>
    void DrawSubMesh(int i);
    /// <summary>
    /// Draws all sub-meshes using MultiDraw. Expects any necessary objects to
    /// be setup and bound as the mesh only binds its VAO
    /// </summary>
    void BatchSubmeshes();

    unsigned int GetVertexCount() const { return vertexCount; }

    unsigned int GetTriCount() const { return GetVertexCount() / 3; }

    int GetSubMeshCount() const { return (int)meshLayers.size(); }

    bool GetSubMesh(int i, const mesh::SubMesh* s) const;
    bool GetSubMesh(const std::string& name, const mesh::SubMesh* s) const;

    static GLuint requiredSize(const mesh::Data& meshData);
    static GLuint findIndexOffset(const mesh::Data& meshData);
    static GLuint calculateVertexStride(const mesh::Data& meshData);

    struct BufferLocation {
      const gl::Id& id;
      const gl::Mapping& mapping;
      GLuint offset;
    };

    Mesh(const engine::mesh::Data& meshData, const BufferLocation& buffer,
         bool write = true);

  protected:
    /// <summary>
    /// Creates a buffer for all the mesh data.
    /// MUST only be called ONCE per mesh, after all data has been set.
    /// </summary>
    void BufferData(const mesh::Data& meshData, const BufferLocation& buffer,
                    GLuint stride);

    /// <summary>
    /// Starting byte offset into the buffer where vertex data starts.
    /// </summary>
    GLuint startOffset = 0;

    /// <summary>
    /// Byte offset into the buffer where index data starts.
    /// </summary>
    GLuint indexOffset = 0;

    gl::Vao vao = {};

    GLuint type = GL_TRIANGLES;

    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;

    std::vector<mesh::SubMesh> meshLayers;
    std::vector<std::string> layerNames;
  };
} // namespace engine
