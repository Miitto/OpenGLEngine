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

#include <array>
#include <gl/gl.hpp>
#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <vector>

namespace engine {

  // A handy enumerator, to determine which member of the bufferObject array
  // holds which data
  enum MeshBuffer {
    VERTEX_BUFFER,
    COLOR_BUFFER,
    TEXTURE_BUFFER,
    NORMAL_BUFFER,
    TANGENT_BUFFER,

    WEIGHTVALUE_BUFFER, // new this year, weight values of vertices
    WEIGHTINDEX_BUFFER, // new this year, indices of weights

    INDEX_BUFFER,

    MAX_BUFFER
  };

  class Mesh {
  public:
    struct SubMesh {
      int start;
      int count;
    };

    Mesh(void) = default;
    ~Mesh(void) = default;
    Mesh(const Mesh& other) = delete;
    Mesh& operator=(const Mesh& other) = delete;
    Mesh(Mesh&& other) = default;
    Mesh& operator=(Mesh&& other) = default;

    void Draw();
    void DrawSubMesh(int i);

    static std::expected<Mesh, std::string>
    LoadFromMeshFile(const std::string& name);

    unsigned int GetVertexCount() const {
      return (unsigned int)vertices.size();
    }

    unsigned int GetTriCount() const {
      size_t primCount = indices.size() > 0 ? indices.size() : vertices.size();
      return static_cast<unsigned int>(primCount / 3);
    }

    unsigned int GetJointCount() const {
      return (unsigned int)jointNames.size();
    }

    int GetIndexForJoint(const std::string& name) const;
    int GetParentForJoint(const std::string& name) const;
    int GetParentForJoint(int i) const;

    const std::vector<glm::mat4>& GetBindPose() const { return bindPose; }

    const std::vector<glm::mat4>& GetInverseBindPose() const {
      return inverseBindPose;
    }

    int GetSubMeshCount() const { return (int)meshLayers.size(); }

    bool GetSubMesh(int i, const SubMesh* s) const;
    bool GetSubMesh(const std::string& name, const SubMesh* s) const;

  protected:
    Mesh(std::vector<glm::vec3>&& vertices, std::vector<glm::vec4>&& colors,
         std::vector<glm::vec2>&& textureCoords,
         std::vector<glm::vec3>&& normals, std::vector<glm::vec4>&& tangents,
         std::vector<glm::vec4>&& weights, std::vector<int>&& weightIndices,
         std::vector<uint32_t>&& indices, std::vector<glm::mat4>&& bindPose,
         std::vector<glm::mat4>&& inverseBindPose,
         std::vector<std::string>&& jointNames, std::vector<int>&& jointParents,
         std::vector<SubMesh>&& meshLayers,
         std::vector<std::string>&& layerNames);
    void BufferData();

    gl::Vao vao = {};
    std::array<std::optional<gl::BasicBuffer>, MAX_BUFFER> bufferObject = {};

    GLuint type = GL_TRIANGLES;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec2> textureCoords;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec4> tangents;

    std::vector<glm::vec4> weights;
    std::vector<int> weightIndices;

    std::vector<uint32_t> indices;

    std::vector<glm::mat4> bindPose;
    std::vector<glm::mat4> inverseBindPose;

    std::vector<std::string> jointNames;
    std::vector<int> jointParents;
    std::vector<SubMesh> meshLayers;
    std::vector<std::string> layerNames;
  };
} // namespace engine
