#include "engine/mesh.hpp"
#include "logger.hpp"
#include <fstream>

using std::string;

namespace engine {
  void Mesh::Draw() {
    auto bg = vao.bindGuard();
    if (indexOffset != 0) {
      uintptr_t offset = static_cast<uintptr_t>(indexOffset);
      glDrawElements(type, indices.size(), GL_UNSIGNED_INT,
                     reinterpret_cast<GLvoid*>(offset));
    } else {
      glDrawArrays(type, 0, vertices.size());
    }
  }

  void Mesh::DrawSubMesh(int i) {
    if (i < 0 || i >= (int)meshLayers.size()) {
      return;
    }
    SubMesh m = meshLayers[i];

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
        auto vertexOffset = subMesh.start * sizeof(unsigned int);
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

  void Mesh::BufferData() {
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
    if (weights.size() > vertexNum) {
      engine::Logger::warn(
          "Mesh::BufferData: weights size greater than vertices size!");
    }
    if (weightIndices.size() > vertexNum) {
      engine::Logger::warn(
          "Mesh::BufferData: weightIndices size greater than vertices size!");
    }

#define SZ(VEC, T) auto VEC##Size = static_cast<GLuint>(VEC.size() * sizeof(T))
    SZ(vertices, glm::vec3);
    SZ(colors, glm::vec4);
    SZ(textureCoords, glm::vec2);
    SZ(normals, glm::vec3);
    SZ(tangents, glm::vec4);
    SZ(weights, glm::vec4);
    SZ(weightIndices, glm::ivec4);
    SZ(indices, unsigned int);
#undef SZ
    indexOffset = verticesSize + colorsSize + textureCoordsSize + normalsSize +
                  tangentsSize + weightsSize + weightIndicesSize;
    uint64_t size = indexOffset + indicesSize;

    buffer.init(size, nullptr, gl::Buffer::Usage::WRITE);

    vao.attribFormat(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    GLuint stride = sizeof(glm::vec3);

#define AC(FIELD) !FIELD.empty()

    if (AC(colors)) {
      vao.attribFormat(1, 4, GL_FLOAT, GL_FALSE, stride, 0);
      stride += sizeof(glm::vec4);
    }
    if (AC(textureCoords)) {
      vao.attribFormat(2, 2, GL_FLOAT, GL_FALSE, stride, 0);
      stride += sizeof(glm::vec2);
    }
    if (AC(normals)) {
      vao.attribFormat(3, 3, GL_FLOAT, GL_FALSE, stride, 0);
      stride += sizeof(glm::vec3);
    }
    if (AC(tangents)) {
      vao.attribFormat(4, 4, GL_FLOAT, GL_FALSE, stride, 0);
      stride += sizeof(glm::vec4);
    }
    if (AC(weights)) {
      vao.attribFormat(5, 4, GL_FLOAT, GL_FALSE, stride, 0);
      stride += sizeof(glm::vec4);
    }
    if (AC(weightIndices)) {
      vao.attribFormat(6, 4, GL_INT, GL_FALSE, stride, 0);
      stride += sizeof(glm::ivec4);
    }
    if (AC(indices)) {
      vao.bindIndexBuffer(buffer.id());
    }

    vao.bindVertexBuffer(0, buffer.id(), 0, stride);

    auto mapping = buffer.map(gl::Buffer::Mapping::WRITE |
                              gl::Buffer::Mapping::INVALIDATE_BUFFER);

#define WRITE(VEC, T)                                                          \
  if (AC(VEC)) {                                                               \
    mapping.write(&VEC[i], sizeof(T), offset);                                 \
    offset += sizeof(T);                                                       \
  }

    GLuint offset = 0;
    for (size_t i = 0; i < vertices.size(); ++i) {
      WRITE(vertices, glm::vec3);
      WRITE(colors, glm::vec4);
      WRITE(textureCoords, glm::vec2);
      WRITE(normals, glm::vec3);
      WRITE(tangents, glm::vec4);
      WRITE(weights, glm::vec4);
      WRITE(weightIndices, glm::ivec4);
    }

    mapping.write(indices.data(), indicesSize, offset);
#undef WRITE
#undef AC
  }

  /*
   *
   * Extra file loading stuff!
   *
   * */

  enum class GeometryChunkTypes {
    VPositions = 1,
    VNormals = 2,
    VTangents = 4,
    VColors = 8,
    VTex0 = 16,
    VTex1 = 32,
    VWeightValues = 64,
    VWeightIndices = 128,
    Indices = 256,
    JointNames = 512,
    JointParents = 1024,
    BindPose = 2048,
    BindPoseInv = 4096,
    Material = 65536,
    SubMeshes = 1 << 14,
    SubMeshNames = 1 << 15
  };

  void ReadTextFloats(std::ifstream& file, std::vector<glm::vec2>& element,
                      int numVertices) {
    for (int i = 0; i < numVertices; ++i) {
      glm::vec2 temp;
      file >> temp.x;
      file >> temp.y;
      element.emplace_back(temp);
    }
  }

  void ReadTextFloats(std::ifstream& file, std::vector<glm::vec3>& element,
                      int numVertices) {
    for (int i = 0; i < numVertices; ++i) {
      glm::vec3 temp;
      file >> temp.x;
      file >> temp.y;
      file >> temp.z;
      element.emplace_back(temp);
    }
  }

  void ReadTextFloats(std::ifstream& file, std::vector<glm::vec4>& element,
                      int numVertices) {
    for (int i = 0; i < numVertices; ++i) {
      glm::vec4 temp;
      file >> temp.x;
      file >> temp.y;
      file >> temp.z;
      file >> temp.w;
      element.emplace_back(temp);
    }
  }

  void ReadTextVertexIndices(std::ifstream& file,
                             std::vector<glm::ivec4>& element,
                             int numVertices) {
    for (int i = 0; i < numVertices; ++i) {
      int indices[4];
      file >> indices[0];
      file >> indices[1];
      file >> indices[2];
      file >> indices[3];
      element.emplace_back(indices[0], indices[1], indices[2], indices[3]);
    }
  }

  void ReadIndices(std::ifstream& file, std::vector<unsigned int>& elements,
                   int numIndices) {
    for (int i = 0; i < numIndices; ++i) {
      unsigned int temp;
      file >> temp;
      elements.emplace_back(temp);
    }
  }

  void ReadJointParents(std::ifstream& file, std::vector<int>& dest) {
    int jointCount = 0;
    file >> jointCount;

    for (int i = 0; i < jointCount; ++i) {
      int id = -1;
      file >> id;
      dest.emplace_back(id);
    }
  }

  void ReadJointNames(std::ifstream& file, std::vector<string>& dest) {
    int jointCount = 0;
    file >> jointCount;
    for (int i = 0; i < jointCount; ++i) {
      std::string jointName;
      file >> jointName;
      dest.emplace_back(jointName);
    }
  }

  void ReadRigPose(std::ifstream& file, std::vector<glm::mat4>& into) {
    int matCount = 0;
    file >> matCount;

    into = std::vector<glm::mat4>(matCount);

    for (int i = 0; i < matCount; ++i) {
      glm::mat4 mat{};
      for (int x = 0; x < 4; ++x) {
        for (int y = 0; y < 4; ++y) {
          file >> mat[x][y];
        }
      }
      into[i] = mat;
    }
  }

  void ReadSubMeshes(std::ifstream& file, int count,
                     std::vector<Mesh::SubMesh>& subMeshes) {
    for (int i = 0; i < count; ++i) {
      Mesh::SubMesh m;
      file >> m.start;
      file >> m.count;
      subMeshes.emplace_back(m);
    }
  }

  void ReadSubMeshNames(std::ifstream& file, int count,
                        std::vector<string>& names) {
    std::string scrap;
    std::getline(file, scrap);

    for (int i = 0; i < count; ++i) {
      std::string meshName;
      std::getline(file, meshName);
      names.emplace_back(meshName);
    }
  }

  std::expected<Mesh, std::string> Mesh::LoadFromMeshFile(const string& name) {
    std::ifstream file(name);
    if (!file.is_open()) {
      engine::Logger::error("Failed to open MeshGeometry file: {}", name);
      return std::unexpected("Failed to open file");
    }

    std::string filetype;
    int fileVersion;

    file >> filetype;

    if (filetype != "MeshGeometry") {
      engine::Logger::error("File is not a MeshGeometry file!");
      return std::unexpected("File is not a MeshGeometry file");
    }

    file >> fileVersion;

    if (fileVersion != 1) {
      engine::Logger::error("MeshGeometry file has incompatible version!");
      return std::unexpected("File has an incompatible version");
    }

    int numMeshes = 0;   // read
    int numVertices = 0; // read
    int numIndices = 0;  // read
    int numChunks = 0;   // read

    file >> numMeshes;
    file >> numVertices;
    file >> numIndices;
    file >> numChunks;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec2> textureCoords;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec4> tangents;

    std::vector<glm::vec4> weights;
    std::vector<glm::ivec4> weightIndices;

    std::vector<uint32_t> indices;

    std::vector<glm::mat4> bindPose;
    std::vector<glm::mat4> inverseBindPose;

    std::vector<std::string> jointNames;
    std::vector<int> jointParents;
    std::vector<SubMesh> meshLayers;
    std::vector<std::string> layerNames;

    for (int i = 0; i < numChunks; ++i) {
      int chunkType = (int)GeometryChunkTypes::VPositions;

      file >> chunkType;

      switch ((GeometryChunkTypes)chunkType) {
      case GeometryChunkTypes::VPositions:
        ReadTextFloats(file, vertices, numVertices);
        break;
      case GeometryChunkTypes::VColors:
        ReadTextFloats(file, colors, numVertices);
        break;
      case GeometryChunkTypes::VNormals:
        ReadTextFloats(file, normals, numVertices);
        break;
      case GeometryChunkTypes::VTangents:
        ReadTextFloats(file, tangents, numVertices);
        break;
      case GeometryChunkTypes::VTex0:
        ReadTextFloats(file, textureCoords, numVertices);
        break;
      case GeometryChunkTypes::Indices:
        ReadIndices(file, indices, numIndices);
        break;

      case GeometryChunkTypes::VWeightValues:
        ReadTextFloats(file, weights, numVertices);
        break;
      case GeometryChunkTypes::VWeightIndices:
        ReadTextVertexIndices(file, weightIndices, numVertices);
        break;
      case GeometryChunkTypes::JointNames:
        ReadJointNames(file, jointNames);
        break;
      case GeometryChunkTypes::JointParents:
        ReadJointParents(file, jointParents);
        break;
      case GeometryChunkTypes::BindPose:
        ReadRigPose(file, bindPose);
        break;
      case GeometryChunkTypes::BindPoseInv:
        ReadRigPose(file, inverseBindPose);
        break;
      case GeometryChunkTypes::SubMeshes:
        ReadSubMeshes(file, numMeshes, meshLayers);
        break;
      case GeometryChunkTypes::SubMeshNames:
        ReadSubMeshNames(file, numMeshes, layerNames);
        break;
      }
    }

#define M(NAME) std::move(NAME)

    Mesh mesh(M(vertices), M(colors), M(textureCoords), M(normals), M(tangents),
              M(weights), M(weightIndices), M(indices), M(bindPose),
              M(inverseBindPose), M(jointNames), M(jointParents), M(meshLayers),
              M(layerNames));

    mesh.BufferData();

    return std::expected<engine::Mesh, std::string>(std::move(mesh));
  }

#define SET(NAME) NAME(std::move(NAME))

  Mesh::Mesh(std::vector<glm::vec3>&& vertices, std::vector<glm::vec4>&& colors,
             std::vector<glm::vec2>&& textureCoords,
             std::vector<glm::vec3>&& normals,
             std::vector<glm::vec4>&& tangents,
             std::vector<glm::vec4>&& weights,
             std::vector<glm::ivec4>&& weightIndices,
             std::vector<uint32_t>&& indices, std::vector<glm::mat4>&& bindPose,
             std::vector<glm::mat4>&& inverseBindPose,
             std::vector<std::string>&& jointNames,
             std::vector<int>&& jointParents, std::vector<SubMesh>&& meshLayers,
             std::vector<std::string>&& layerNames)
      : SET(vertices), SET(colors), SET(textureCoords), SET(normals),
        SET(tangents), SET(weights), SET(weightIndices), SET(indices),
        SET(bindPose), SET(inverseBindPose), SET(jointNames), SET(jointParents),
        SET(meshLayers), SET(layerNames) {}

#undef SET

  int Mesh::GetIndexForJoint(const std::string& name) const {
    for (unsigned int i = 0; i < jointNames.size(); ++i) {
      if (jointNames[i] == name) {
        return i;
      }
    }
    return -1;
  }

  int Mesh::GetParentForJoint(const std::string& name) const {
    int i = GetIndexForJoint(name);
    if (i == -1) {
      return -1;
    }
    return jointParents[i];
  }

  int Mesh::GetParentForJoint(int i) const {
    if (i == -1 || i >= (int)jointParents.size()) {
      return -1;
    }
    return jointParents[i];
  }

  bool Mesh::GetSubMesh(int i, const SubMesh* s) const {
    if (i < 0 || i >= (int)meshLayers.size()) {
      return false;
    }
    s = &meshLayers[i];
    return true;
  }

  bool Mesh::GetSubMesh(const string& name, const SubMesh* s) const {
    for (unsigned int i = 0; i < layerNames.size(); ++i) {
      if (layerNames[i] == name) {
        return GetSubMesh(i, s);
      }
    }
    return false;
  }
} // namespace engine