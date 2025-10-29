#pragma once

#include "plane.hpp"
#include "scene_node.hpp"
#include <glm/glm.hpp>

namespace engine {
  class Frustum {
  public:
    struct Planes {
      Plane n;
      Plane f;
      Plane left;
      Plane right;
      Plane top;
      Plane bottom;

      inline bool SphereInAllPlanes(const glm::vec3& centre,
                                    float radius) const {
        bool nr = n.SphereInPlane(centre, radius);
        bool fr = f.SphereInPlane(centre, radius);
        bool lr = left.SphereInPlane(centre, radius);
        bool rr = right.SphereInPlane(centre, radius);
        bool tr = top.SphereInPlane(centre, radius);
        bool br = bottom.SphereInPlane(centre, radius);

        return nr && fr && lr && rr && tr && br;
      }
    };

    Frustum(const glm::mat4& mat);
    ~Frustum() = default;

    inline bool SphereInFrustum(const glm::vec3& centre, float radius) const {
      return m_planes.SphereInAllPlanes(centre, radius);
    }

    inline bool NodeInFrustum(const engine::scene::Node& node) const {
      auto& world = node.GetTransforms().world;
      auto pos = world[3];
      return SphereInFrustum(pos, node.GetBoundingRadius());
    }

  protected:
    Planes m_planes;
  };
} // namespace engine
