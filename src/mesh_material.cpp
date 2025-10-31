#include "engine/mesh_material.hpp"
#include <fstream>
#include <iostream>

#include "logger.hpp"

using std::ifstream;

namespace engine::mesh {
  Material::Material(const std::string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
      Logger::error("Could not open mesh material file: {}", filename);
      return;
    }

    std::string dataType;
    file >> dataType;

    if (dataType != "MeshMat") {
      engine::Logger::error(
          "Loading mesh material from file: {}. Not a Mat file", filename);
      return;
    }
    int version;
    file >> version;

    if (version != 1) {
      engine::Logger::error("Loading mesh material from file: {}. Unsupported "
                            "Mat version: {}",
                            filename, version);
      return;
    }

    int matCount;
    int meshCount;
    file >> matCount;
    file >> meshCount;

    materialLayers.resize(matCount);

    for (int i = 0; i < matCount; ++i) {
      std::string name;
      int count;

      file >> name;
      file >> count;

      for (int j = 0; j < count; ++j) {
        std::string entryData;
        file >> entryData;
        std::string channel;
        std::string file;
        size_t split = entryData.find_first_of(':');
        channel = entryData.substr(0, split);
        file = entryData.substr(split + 1);

        materialLayers[i].entries.insert(std::make_pair(channel, file));
      }
    }

    for (int i = 0; i < meshCount; ++i) {
      int entry;
      file >> entry;
      meshLayers.emplace_back(&materialLayers[entry]);
    }
  }

  const MaterialEntry* Material::GetMaterialForLayer(int i) const {
    if (i < 0 || i >= (int)meshLayers.size()) {
      return nullptr;
    }
    return meshLayers[i];
  }
} // namespace engine::mesh