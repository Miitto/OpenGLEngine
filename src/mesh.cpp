#include "engine/mesh.hpp"
#include "logger.hpp"
#include <fstream>

using std::string;

namespace engine {
  void Mesh::Draw() {
    auto bg = vao.bindGuard();

    if (bufferObject[INDEX_BUFFER]) {
      glDrawElements(type, indices.size(), GL_UNSIGNED_INT, 0);
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
    if (bufferObject[INDEX_BUFFER]) {
      const GLvoid* offset = (const GLvoid*)(m.start * sizeof(unsigned int));
      glDrawElements(type, m.count, GL_UNSIGNED_INT, offset);
    } else {
      glDrawArrays(type, m.start, m.count); // Draw the triangle!
    }
    vao.unbind();
  }

  gl::BasicBuffer UploadAttribute(const gl::Vao& vao, int numElements,
                                  int dataSize, int attribSize, int attribID,
                                  void* pointer, const string& debugName) {
    using gl::Buffer;
    gl::BasicBuffer buf(numElements * dataSize, pointer,
                        Buffer::Usage::DEFAULT);
    vao.bindVertexBuffer(attribID, buf.id(), 0, dataSize);
    vao.attribFormat(attribID, attribSize, GL_FLOAT, GL_FALSE, 0, attribID);

    buf.label(debugName.c_str());

    return buf;
  }

  void Mesh::BufferData() {

    int numVertices = static_cast<int>(vertices.size());

    ////Buffer vertex data
    bufferObject[VERTEX_BUFFER] =
        UploadAttribute(vao, numVertices, sizeof(glm::vec3), 3, VERTEX_BUFFER,
                        vertices.data(), "Positions");

    if (!colors.empty()) {
      bufferObject[COLOR_BUFFER] =
          UploadAttribute(vao, numVertices, sizeof(glm::vec4), 4, COLOR_BUFFER,
                          colors.data(), "Colors");
    }

    if (!textureCoords.empty()) { // Buffer texture data
      bufferObject[TEXTURE_BUFFER] =
          UploadAttribute(vao, numVertices, sizeof(glm::vec2), 2,
                          TEXTURE_BUFFER, textureCoords.data(), "TexCoords");
    }

    if (!normals.empty()) { // Buffer normal data
      bufferObject[NORMAL_BUFFER] =
          UploadAttribute(vao, numVertices, sizeof(glm::vec3), 3, NORMAL_BUFFER,
                          normals.data(), "Normals");
    }

    if (!tangents.empty()) { // Buffer tangent data
      bufferObject[TANGENT_BUFFER] =
          UploadAttribute(vao, numVertices, sizeof(glm::vec4), 4,
                          TANGENT_BUFFER, tangents.data(), "Tangents");
    }

    if (!weights.empty()) { // Buffer weights data
      bufferObject[WEIGHTVALUE_BUFFER] =
          UploadAttribute(vao, numVertices, sizeof(glm::vec4), 4,
                          WEIGHTVALUE_BUFFER, weights.data(), "Weights");
    }

    // Buffer weight indices data...uses a different function since its
    // integers...
    if (!weightIndices.empty()) {
      bufferObject[WEIGHTINDEX_BUFFER] = UploadAttribute(
          vao, numVertices, sizeof(int) * 4, 4, WEIGHTINDEX_BUFFER,
          weightIndices.data(), "Weight Indices");
    }

    // buffer index data
    if (!indices.empty()) {
      bufferObject[INDEX_BUFFER] = gl::BasicBuffer(
          static_cast<GLuint>(indices.size() * sizeof(unsigned int)),
          indices.data(), gl::Buffer::Usage::DEFAULT);
      bufferObject[INDEX_BUFFER]->label("Indices");
      vao.bindIndexBuffer(bufferObject[INDEX_BUFFER]->id());
    }
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

  void ReadTextVertexIndices(std::ifstream& file, std::vector<int>& element,
                             int numVertices) {
    for (int i = 0; i < numVertices; ++i) {
      int indices[4];
      file >> indices[0];
      file >> indices[1];
      file >> indices[2];
      file >> indices[3];
      element.emplace_back(indices[0]);
      element.emplace_back(indices[1]);
      element.emplace_back(indices[2]);
      element.emplace_back(indices[3]);
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
      glm::mat4 mat;
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++i) {
          file >> mat[i][j];
        }
      }
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

  Mesh* Mesh::LoadFromMeshFile(const string& name) {
    Mesh* mesh = new Mesh();

    std::ifstream file(name);

    std::string filetype;
    int fileVersion;

    file >> filetype;

    if (filetype != "MeshGeometry") {
      engine::Logger::error("File is not a MeshGeometry file!");
      return nullptr;
    }

    file >> fileVersion;

    if (fileVersion != 1) {
      engine::Logger::error("MeshGeometry file has incompatible version!");
      return nullptr;
    }

    int numMeshes = 0;   // read
    int numVertices = 0; // read
    int numIndices = 0;  // read
    int numChunks = 0;   // read

    file >> numMeshes;
    file >> numVertices;
    file >> numIndices;
    file >> numChunks;

    for (int i = 0; i < numChunks; ++i) {
      int chunkType = (int)GeometryChunkTypes::VPositions;

      file >> chunkType;

      switch ((GeometryChunkTypes)chunkType) {
      case GeometryChunkTypes::VPositions:
        ReadTextFloats(file, mesh->vertices, numVertices);
        break;
      case GeometryChunkTypes::VColors:
        ReadTextFloats(file, mesh->colors, numVertices);
        break;
      case GeometryChunkTypes::VNormals:
        ReadTextFloats(file, mesh->normals, numVertices);
        break;
      case GeometryChunkTypes::VTangents:
        ReadTextFloats(file, mesh->tangents, numVertices);
        break;
      case GeometryChunkTypes::VTex0:
        ReadTextFloats(file, mesh->textureCoords, numVertices);
        break;
      case GeometryChunkTypes::Indices:
        ReadIndices(file, mesh->indices, numIndices);
        break;

      case GeometryChunkTypes::VWeightValues:
        ReadTextFloats(file, mesh->weights, numVertices);
        break;
      case GeometryChunkTypes::VWeightIndices:
        ReadTextVertexIndices(file, mesh->weightIndices, numVertices);
        break;
      case GeometryChunkTypes::JointNames:
        ReadJointNames(file, mesh->jointNames);
        break;
      case GeometryChunkTypes::JointParents:
        ReadJointParents(file, mesh->jointParents);
        break;
      case GeometryChunkTypes::BindPose:
        ReadRigPose(file, mesh->bindPose);
        break;
      case GeometryChunkTypes::BindPoseInv:
        ReadRigPose(file, mesh->inverseBindPose);
        break;
      case GeometryChunkTypes::SubMeshes:
        ReadSubMeshes(file, numMeshes, mesh->meshLayers);
        break;
      case GeometryChunkTypes::SubMeshNames:
        ReadSubMeshNames(file, numMeshes, mesh->layerNames);
        break;
      }
    }

    mesh->BufferData();

    return mesh;
  }

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