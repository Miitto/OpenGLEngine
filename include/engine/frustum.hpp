#pragma once

#include "plane.hpp"
#include "scene_node.hpp"
#include <glm/glm.hpp>

namespace engine {
  /// <summary>
  /// Represents a view frustum defined by six planes.
  /// </summary>
  class Frustum {
  public:
    /// <summary>
    /// Each of the six planes that define the frustum.
    /// </summary>
    struct Planes {
      Plane n;
      Plane f;
      Plane left;
      Plane right;
      Plane top;
      Plane bottom;

      /// <summary>
      /// Returns true if a sphere is inside all six planes of the frustum.
      /// </summary>
      /// <param name="centre">Sphere center</param>
      /// <param name="radius">Sphere radius</param>
      /// <returns>Whether the sphere is in all six planes</returns>
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

    /// <summary>
    /// Creates a frustum from a combined view-projection matrix.
    /// </summary>
    /// <param name="mat">View projection matrix</param>
    Frustum(const glm::mat4& mat);
    ~Frustum() = default;

    /// <summary>
    /// Returns true if a sphere is inside the frustum.
    /// </summary>
    /// <param name="centre">Sphere center</param>
    /// <param name="radius">Sphere radius</param>
    /// <returns>Whether the sphere is inside the frustum</returns>
    inline bool SphereInFrustum(const glm::vec3& centre, float radius) const {
      return m_planes.SphereInAllPlanes(centre, radius);
    }

    /// <summary>
    /// Returns true if a scene node is inside the frustum.
    /// </summary>
    /// <param name="node">Scene node</param>
    /// <returns>Whether the node is inside the frustum</returns>
    inline bool NodeInFrustum(const engine::scene::Node& node) const {
      auto& world = node.GetTransforms().world;
      auto pos = world[3];
      return SphereInFrustum(pos, node.GetBoundingRadius());
    }

  protected:
    Planes m_planes;
  };
} // namespace engine
