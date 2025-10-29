#include "engine/frustum.hpp"

namespace engine {
  Frustum::Frustum(const glm::mat4& mat) {
    glm::vec3 x = {mat[0][0], mat[1][0], mat[2][0]};
    glm::vec3 y = {mat[0][1], mat[1][1], mat[2][1]};
    glm::vec3 z = {mat[0][2], mat[1][2], mat[2][2]};
    glm::vec3 w = {mat[0][3], mat[1][3], mat[2][3]};

    float d = mat[3][3];
    float xd = mat[3][0];
    float yd = mat[3][1];
    float zd = mat[3][2];

    m_planes.right = Plane{w - x, d - xd, true};
    m_planes.left = Plane{w + x, d + xd, true};
    m_planes.bottom = Plane{w + y, d + yd, true};
    m_planes.top = Plane{w - y, d - yd, true};
    m_planes.f = Plane{w - z, d - zd, true};
    m_planes.n = Plane{w + z, d + zd, true};
  }

} // namespace engine