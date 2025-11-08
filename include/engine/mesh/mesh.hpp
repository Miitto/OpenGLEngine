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

  struct WeightedVertex {
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;
    glm::vec4 tangent;
    glm::vec4 jointWeights;
    glm::ivec4 jointIndices;
  };

  struct Vertex {
    glm::vec3 position;
    glm::vec4 color;
    glm::vec2 texCoord;
    glm::vec3 normal;
    glm::vec4 tangent;
  };

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
    explicit Mesh(const Mesh& other) = default;
    Mesh& operator=(const Mesh& other) = default;
    Mesh(Mesh&& other) = default;
    Mesh& operator=(Mesh&& other) = default;

    /// <summary>
    /// Draws all sub-meshes using MultiDraw. Expects any necessary objects to
    /// be setup and bound as the mesh only binds its VAO
    /// Takes in the buffer and offset that should have just been used with
    /// prepSubmeshesForBatch
    /// </summary>
    void BatchSubmeshes(GLuint offset);

    GLuint writeBatchedDraws(gl::MappingRef& mapping, GLuint baseVertex,
                             GLuint instances, GLuint baseInstance) const;

    unsigned int GetVertexCount() const { return vertexCount; }

    unsigned int GetTriCount() const { return GetVertexCount() / 3; }

    unsigned int GetSubMeshCount() const {
      return (unsigned int)meshLayers.size();
    }

    bool GetSubMesh(int i, const mesh::SubMesh* s) const;
    bool GetSubMesh(const std::string& name, const mesh::SubMesh* s) const;

    constexpr static GLuint vertexStride() {
      return sizeof(glm::vec3) + sizeof(glm::vec4) + sizeof(glm::vec2) +
             sizeof(glm::vec3) + sizeof(glm::vec4) + sizeof(glm::vec4) +
             sizeof(glm::ivec4);
    }

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
                         GLuint& vertexStartIndex,
                         const gl::MappingRef stagingMapping);

    void writeIndexData(const engine::mesh::Data& meshData, GLuint& indexOffset,
                        const gl::MappingRef stagingMapping);

    void writeJointData(const engine::mesh::Data& meshData,
                        const engine::mesh::Animation& animation,
                        const gl::MappingRef stagingMapping,
                        uint32_t jointStartIndex);

    GLuint getVertexOffset() const { return vertexOffset; }
    GLuint getFrameCount() const { return frameCount; }
    GLuint getJointCount() const { return jointCount; }
    GLuint getStartJointIndex() const { return startJointIndex; }
    float getOneOverFrameRate() const { return oneOverFrameRate; }

  protected:
    GLuint vertexOffset = 0;

    /// <summary>
    /// Number of indices into the buffer where index data starts.
    /// Same as the byte offset / sizeof(uint32_t)
    /// </summary>
    GLuint indexOffset = 0;

    /// <summary>
    /// Index of the first joint for this mesh (relative to the other joints in
    /// its buffer).
    /// </summary>
    GLuint startJointIndex = 0;

    GLuint frameCount = 0;
    GLuint jointCount = 0;
    float oneOverFrameRate = 0.0f;

    GLuint type = GL_TRIANGLES;

    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
    uint32_t instanceCount = 1;

    std::vector<mesh::SubMesh> meshLayers;
    std::vector<std::string> layerNames;
  };
} // namespace engine::mesh
