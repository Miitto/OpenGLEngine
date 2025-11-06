#pragma once

#include <expected>
#include <gl/texture.hpp>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace engine::mesh {
  struct TextureImageSet {
    gl::Texture diffuse;
    std::optional<gl::Texture> bump = std::nullopt;
    std::optional<gl::Texture> material = std::nullopt;
  };

  struct TextureHandleSet {
    gl::RawTextureHandle diffuse;
    gl::RawTextureHandle bump = 0;
    gl::RawTextureHandle material = 0;
  };

  struct TextureSet {
    TextureImageSet images;
    TextureHandleSet handles;
  };

  class MaterialEntry {
  public:
    std::map<std::string, std::string> entries;

    std::optional<std::string_view> GetEntry(const std::string& name) const {
      auto i = entries.find(name);
      if (i == entries.end()) {
        return std::nullopt;
      }
      return i->second;
    }

    std::expected<TextureSet, std::string>
    LoadTextures(const std::string& basePath) const;
  };

  class Material {
  public:
    Material(const std::string& filename);
    ~Material() {}
    const MaterialEntry* GetMaterialForLayer(int i) const;

  protected:
    std::vector<MaterialEntry> materialLayers;
    std::vector<MaterialEntry*> meshLayers;
  };

} // namespace engine::mesh
