#pragma once

#include <expected>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace engine::mesh {
  struct SubMesh {
    int start;
    int count;
  };

  class Data {
  public:
    /// <summary>
    /// Loads a .msh file from disk.
    /// </summary>
    /// <param name="name">File path</param>
    /// <returns>Mesh on success, error string on failure</returns>
    static std::expected<Data, std::string>
    fromFile(const std::string_view& name);

    // static std::expected<Data, std::string>
    // fromGLTFFile(const std::string_view& name);

    Data(std::vector<glm::vec3>&& vertices, std::vector<glm::vec4>&& colors,
         std::vector<glm::vec2>&& textureCoords,
         std::vector<glm::vec3>&& normals, std::vector<glm::vec4>&& tangents,
         std::vector<glm::vec4>&& weights,
         std::vector<glm::ivec4>&& weightIndices,
         std::vector<uint32_t>&& indices, std::vector<glm::mat4>&& bindPose,
         std::vector<glm::mat4>&& inverseBindPose,
         std::vector<std::string>&& jointNames, std::vector<int>&& jointParents,
         std::vector<SubMesh>&& meshLayers,
         std::vector<std::string>&& layerNames);

    const std::vector<glm::vec3>& vertices() const { return _vertices; }
    const std::vector<glm::vec4>& colors() const { return _colors; }
    const std::vector<glm::vec2>& textureCoords() const {
      return _textureCoords;
    }
    const std::vector<glm::vec3>& normals() const { return _normals; }
    const std::vector<glm::vec4>& tangents() const { return _tangents; }
    const std::vector<glm::vec4>& weights() const { return _weights; }
    const std::vector<glm::ivec4>& weightIndices() const {
      return _weightIndices;
    }
    const std::vector<uint32_t>& indices() const { return _indices; }
    const std::vector<glm::mat4>& bindPose() const { return _bindPose; }
    const std::vector<glm::mat4>& inverseBindPose() const {
      return _inverseBindPose;
    }
    const std::vector<std::string>& jointNames() const { return _jointNames; }
    const std::vector<int>& jointParents() const { return _jointParents; }
    const std::vector<SubMesh>& meshLayers() const { return _meshLayers; }
    const std::vector<std::string>& layerNames() const { return _layerNames; }

  protected:
    GLuint type = GL_TRIANGLES;

    std::vector<glm::vec3> _vertices = {};
    std::vector<glm::vec4> _colors = {};
    std::vector<glm::vec2> _textureCoords = {};
    std::vector<glm::vec3> _normals = {};
    std::vector<glm::vec4> _tangents = {};

    std::vector<glm::vec4> _weights = {};
    std::vector<glm::ivec4> _weightIndices = {};

    std::vector<uint32_t> _indices = {};

    std::vector<glm::mat4> _bindPose = {};
    std::vector<glm::mat4> _inverseBindPose = {};

    std::vector<std::string> _jointNames = {};
    std::vector<int> _jointParents = {};
    std::vector<SubMesh> _meshLayers = {};
    std::vector<std::string> _layerNames = {};
  };
} // namespace engine::mesh