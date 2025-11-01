#include "engine/mesh/mesh_data.hpp"

#include "../logger.hpp"
#include <fstream>

namespace {
  using engine::mesh::SubMesh;

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

  void ReadJointNames(std::ifstream& file, std::vector<std::string>& dest) {
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
                     std::vector<SubMesh>& subMeshes) {
    for (int i = 0; i < count; ++i) {
      SubMesh m;
      file >> m.start;
      file >> m.count;
      subMeshes.emplace_back(m);
    }
  }

  void ReadSubMeshNames(std::ifstream& file, int count,
                        std::vector<std::string>& names) {
    std::string scrap;
    std::getline(file, scrap);

    for (int i = 0; i < count; ++i) {
      std::string meshName;
      std::getline(file, meshName);
      names.emplace_back(meshName);
    }
  }
} // namespace

namespace engine::mesh {
  std::expected<Data, std::string>
  Data::fromFile(const std::string_view& name) {
    std::ifstream file(name.data());
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

    Data mesh(M(vertices), M(colors), M(textureCoords), M(normals), M(tangents),
              M(weights), M(weightIndices), M(indices), M(bindPose),
              M(inverseBindPose), M(jointNames), M(jointParents), M(meshLayers),
              M(layerNames));

    return std::expected<Data, std::string>(std::move(mesh));
  }

#define SET(NAME) _##NAME(std::move(NAME))

  Data::Data(std::vector<glm::vec3>&& vertices, std::vector<glm::vec4>&& colors,
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

} // namespace engine::mesh