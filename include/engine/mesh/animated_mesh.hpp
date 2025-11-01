#pragma once
// Modified Mesh class, originally created by Rich Davison. See mesh.hpp for
// original (ish).

#include "engine/mesh/mesh.hpp" // <-- Original
#include "engine/mesh/mesh_animation.hpp"
#include <array>
#include <gl/gl.hpp>
#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <vector>

namespace engine {

  // TODO: Mesh class that uses an externally owned buffer with an offset

  /// <summary>
  /// Mesh class that supports skeletal animation.
  /// This mesh owns one buffer that holds all mesh data.
  /// </summary>
  /// <remarks>
  /// Attribute Layout:
  /// 0. vec3 position
  /// 1. vec4 color
  /// 2. vec2 texture coordinate
  /// 3. vec3 normal
  /// 4. vec4 tangent
  /// 5. vec4 weights
  /// 6. ivec4 weight indices
  /// </remarks>
  class AnimatedMesh : public Mesh {
  public:
    AnimatedMesh(void) = default;
    ~AnimatedMesh(void) = default;
    AnimatedMesh(const Mesh& other) = delete;
    AnimatedMesh& operator=(const Mesh& other) = delete;
    AnimatedMesh(AnimatedMesh&& other) = default;
    AnimatedMesh& operator=(AnimatedMesh&& other) = default;

    int GetIndexForJoint(const std::string& name) const;
    int GetParentForJoint(const std::string& name) const;
    int GetParentForJoint(int i) const;

    static GLuint requiredSize(const mesh::Data& meshData,
                               const mesh::Animation& animation);
    static GLuint findIndexOffset(const mesh::Data& meshData);
    static GLuint calculateVertexStride(const mesh::Data& meshData);
    static GLuint calculateJointDataSize(const mesh::Animation& animation);

    inline const GLuint getJointOffset() const { return jointOffset; }
    inline const GLuint getJointSize() const { return jointSize; }

    AnimatedMesh(const engine::mesh::Data& meshData,
                 engine::Mesh::BufferLocation buffer,
                 const mesh::Animation& animation);

  protected:
    /// <summary>
    /// Creates a buffer for all the mesh data.
    /// MUST only be called ONCE per mesh, after all data has been set.
    /// </summary>
    void BufferData(const mesh::Data& meshData,
                    const Mesh::BufferLocation& buffer, GLuint stride,
                    const mesh::Animation& animation);

    GLuint jointOffset = 0;
    GLuint jointSize = 0;
  };
} // namespace engine