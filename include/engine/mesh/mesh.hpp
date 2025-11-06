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

#include "engine/mesh/mesh_animation.hpp"
#include "engine/mesh/mesh_data.hpp"
#include <array>
#include <gl/gl.hpp>
#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <vector>

namespace engine::mesh {
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
    /// Prepares an buffer by populating it with indirect draw commands for all
    /// submeshes
    /// </summary>
    /// <param name="indirectBufferMapping">Mapping to write the indirect draw
    /// commands to</param> <param name="offset">Offset into the mapping to
    /// write</param> <param name="instances">Number of instances to
    /// render</param> <param name="baseInstance">Base Instance</param>
    void prepSubmeshesForBatch(const gl::Mapping& indirectBufferMapping,
                               GLuint offset, uint32_t instances,
                               uint32_t baseInstance) const;

    /// <summary>
    /// Draws all sub-meshes using MultiDraw. Expects any necessary objects to
    /// be setup and bound as the mesh only binds its VAO
    /// Takes in the buffer and offset that should have just been used with
    /// prepSubmeshesForBatch
    /// </summary>
    void BatchSubmeshes(const gl::Buffer& buffer, GLuint offset);

    unsigned int GetVertexCount() const { return vertexCount; }

    unsigned int GetTriCount() const { return GetVertexCount() / 3; }

    int GetSubMeshCount() const { return (int)meshLayers.size(); }

    bool GetSubMesh(int i, const mesh::SubMesh* s) const;
    bool GetSubMesh(const std::string& name, const mesh::SubMesh* s) const;

    constexpr static GLuint vertexStride() {
      return sizeof(glm::vec3) + sizeof(glm::vec4) + sizeof(glm::vec2) +
             sizeof(glm::vec3) + sizeof(glm::vec4) + sizeof(glm::vec4) +
             sizeof(glm::ivec4);
    }

    void setupInstanceAttr(GLuint index, GLuint size, GLenum type,
                           GLboolean normalize, GLuint offset,
                           GLuint bufferIndex);
    void bindInstanceBuffer(GLuint index, const gl::Buffer& buffer,
                            GLuint offset, GLuint stride, GLuint divisor = 1);

    struct BufferLocation {
      const gl::Buffer& buffer;
      GLuint offset;
    };

    Mesh(const engine::mesh::Data& meshData);

    static GLuint vertexDataSize(const mesh::Data& meshData);

    struct AlignedSize {
      /// <summary>
      /// Offset where the data should start
      /// </summary>
      GLuint offset;
      /// <summary>
      /// Total size to needed including any alignment padding
      /// This should be used when calculating buffer sizes
      /// </summary>
      GLuint size;
      /// <summary>
      /// Size of the data without any alignment padding
      /// This should be used when binding buffers
      /// </summary>
      GLuint alignedSize;
    };
    static AlignedSize indexDataSize(const mesh::Data& meshData,
                                     GLuint startOffset);

    static AlignedSize jointDataSize(const mesh::Animation& animation,
                                     GLuint startOffset);

    GLuint indirectBufferSize() const;

    void writeVertexData(const engine::mesh::Data& meshData,
                         const BufferLocation& buffer,
                         const gl::MappingRef stagingMapping);

    void writeIndexData(const engine::mesh::Data& meshData,
                        const BufferLocation& buffer,
                        const gl::MappingRef stagingMapping);

    void writeJointData(const engine::mesh::Data& meshData,
                        const engine::mesh::Animation& animation,
                        const BufferLocation& buffer, GLuint jointBindPoint,
                        const gl::MappingRef stagingMapping);

  protected:
    /// <summary>
    /// Number of indices into the buffer where index data starts.
    /// Same as the byte offset / sizeof(uint32_t)
    /// </summary>
    GLuint indexOffset = 0;

    /// <summary>
    /// Byte offset into the buffer where joint data starts.
    /// </summary>
    GLuint jointOffset = 0;

    /// <summary>
    /// Size of the joint data in bytes.
    /// </summary>
    GLuint jointSize = 0;

    /// <summary>
    /// Reference to the buffer holding the joint data
    /// </summary>
    gl::Id jointBuffer = 0;

    /// <summary>
    /// Bind point to bind the joint data buffer to.
    /// </summary>
    GLuint jointBindPoint = 0;

    gl::Vao vao = {};

    GLuint type = GL_TRIANGLES;

    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
    uint32_t instanceCount = 1;

    std::vector<mesh::SubMesh> meshLayers;
    std::vector<std::string> layerNames;
  };
} // namespace engine::mesh
