#pragma once

#include <glm/glm.hpp>

namespace engine {
  class Plane {
  public:
    Plane() = default;
    constexpr inline Plane(const glm::vec3& normal, float d,
                           bool normalize = false) {
      if (normalize) {
        float length = static_cast<float>(normal.length());
        this->normal = normal / length;
        this->d = d / length;
      } else {
        this->normal = normal;
        this->d = d;
      }
    }
    ~Plane() = default;

    inline void SetNormal(const glm::vec3& normal) { this->normal = normal; }
    inline const glm::vec3& GetNormal() const { return normal; }
    inline void SetDistance(float d) { this->d = d; }
    inline float GetDistance() const { return d; }

    inline float SignedDistanceTo(const glm::vec3& point) const {
      return glm::dot(normal, point) + d;
    }

    inline bool SphereInPlane(const glm::vec3& center, float radius) const {
      auto dist = SignedDistanceTo(center);
      return dist > -radius;
    }

  protected:
    glm::vec3 normal;
    float d;
  };
} // namespace engine
