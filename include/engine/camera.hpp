#pragma once
#include "engine/frustum.hpp"
#include "engine/input.hpp"
#include <glm/glm.hpp>

namespace engine {
  class Camera {
  public:
    struct Rotation {
      float pitch;
      float yaw;
    };

    inline Camera() : rotation({0.0, 0.0}), position({}) {}

    inline Camera(Rotation rotation, const glm::vec3& position)
        : rotation(rotation), position(position) {}

    ~Camera() = default;

    virtual void update(const Input& input, float dt);

    glm::mat4 BuildViewMatrix() const;

    inline const glm::vec3& GetPosition() const { return position; };
    inline void SetPosition(const glm::vec3& pos) { position = pos; };
    inline const Rotation& GetRotation() const { return rotation; };
    inline void SetRotation(const Rotation& rot) { rotation = rot; };

    virtual const Frustum& GetFrustum() const = 0;

    void CameraDebugUI() const;

  protected:
    Rotation rotation;
    glm::vec3 position;
  };

  class PerspectiveCamera : public Camera {
  public:
    PerspectiveCamera(float nearPlane, float farPlane, float aspectRatio,
                      float fov);
    ~PerspectiveCamera() = default;

    void update(const Input& input, float dt) override;

    const Frustum& GetFrustum() const override { return m_frustum; };

  protected:
    glm::mat4 m_projMatrix;
    Frustum m_frustum;
  };
} // namespace engine
