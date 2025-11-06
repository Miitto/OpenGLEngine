#include "engine/mesh/mesh_material.hpp"
#include <engine/image.hpp>
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

  std::expected<TextureSet, std::string>
  MaterialEntry::LoadTextures(const std::string& basePath) const {
    auto diffuseOpt = GetEntry("Diffuse");
    if (!diffuseOpt) {
      return std::unexpected("No diffuse texture specified in material");
    }
    auto& diffusePath = *diffuseOpt;
    auto imgOpt = engine::Image::fromFile(basePath + diffusePath.data(), true);
    if (!imgOpt) {
      return std::unexpected("Failed to load diffuse texture from " +
                             std::string(basePath) + std::string(diffusePath));
    }

    auto& img = imgOpt.value();
    auto diffuseTex = img.toTexture(-1);
    diffuseTex.setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    diffuseTex.setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    diffuseTex.createHandle();
    auto diffuseHandle = diffuseTex.handle();

    TextureImageSet images{
        .diffuse = std::move(diffuseTex),
    };

    TextureHandleSet handles{
        .diffuse = diffuseHandle,
    };

    TextureSet textureSet{
        .images = std::move(images),
        .handles = std::move(handles),
    };

    auto bumpOpt = GetEntry("Bump");
    if (bumpOpt) {
      auto& bumpPath = *bumpOpt;
      auto imgOpt = engine::Image::fromFile(basePath + bumpPath.data(), true);
      if (!imgOpt) {
        return std::unexpected("Failed to load bump texture from " +
                               std::string(basePath) + std::string(bumpPath));
      }
      auto& img = imgOpt.value();
      textureSet.images.bump = img.toTexture();
      textureSet.images.bump->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      textureSet.images.bump->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      textureSet.images.bump->createHandle();
      textureSet.handles.bump = textureSet.images.bump->handle();
    }
    auto materialOpt = GetEntry("Material");
    if (materialOpt) {
      auto& materialPath = *materialOpt;
      auto imgOpt =
          engine::Image::fromFile(basePath + materialPath.data(), true);
      if (!imgOpt) {
        return std::unexpected("Failed to load material texture from " +
                               std::string(basePath) +
                               std::string(materialPath));
      }
      auto& img = imgOpt.value();
      textureSet.images.material = img.toTexture(-1);
      textureSet.images.material->setParameter(GL_TEXTURE_MIN_FILTER,
                                               GL_LINEAR_MIPMAP_LINEAR);
      textureSet.images.material->setParameter(GL_TEXTURE_MAG_FILTER,
                                               GL_LINEAR);
      textureSet.images.material->createHandle();
      textureSet.handles.material = textureSet.images.material->handle();
    }
    return textureSet;
  }
} // namespace engine::mesh