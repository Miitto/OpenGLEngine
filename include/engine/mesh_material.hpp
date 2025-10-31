#pragma once
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace engine::mesh {
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
