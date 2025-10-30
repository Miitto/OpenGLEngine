#pragma once
#include <map>
#include <string>
#include <vector>

using std::map;
using std::string;
using std::vector;

namespace engine::mesh {
  class MaterialEntry {
  public:
    std::map<string, string> entries;

    bool GetEntry(const string& name, const string** output) const {
      auto i = entries.find(name);
      if (i == entries.end()) {
        return false;
      }
      *output = &i->second;
      return true;
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
