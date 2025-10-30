#pragma once
#include "engine/frustum.hpp"
#include "engine/input.hpp"
#include <gl/buffer.hpp>
#include <glm/glm.hpp>

namespace engine {
  class Camera {
  public:
    struct Rotation {
      float pitch;
      float yaw;
    };

    struct Matrices {
      glm::mat4 view = glm::mat4(1.0);
      glm::mat4 proj = glm::mat4(1.0);
      glm::mat4 viewProj = glm::mat4(1.0);
      glm::mat4 invView = glm::mat4(1.0);
      glm::mat4 invProj = glm::mat4(1.0);
      glm::mat4 invViewProj = glm::mat4(1.0);
    };

    Camera();
    Camera(Rotation rotation, const glm::vec3& position);
    ~Camera() = default;

    virtual void update(const Input& input, float dt);

    inline const Matrices& GetMatrices() const { return matrices; }
    inline const glm::vec3& GetPosition() const { return position; }
    inline void SetPosition(const glm::vec3& pos) { position = pos; }
    inline const Rotation& GetRotation() const { return rotation; }
    inline void SetRotation(const Rotation& rot) { rotation = rot; }

    inline glm::vec3 forward() const {
      glm::vec3 forward;
      forward.x = -std::sin(glm::radians(rotation.yaw)) *
                  std::cos(glm::radians(rotation.pitch));
      forward.y = std::sin(glm::radians(rotation.pitch));
      forward.z = -std::cos(glm::radians(rotation.yaw)) *
                  std::cos(glm::radians(rotation.pitch));
      return glm::normalize(forward);
    }

    inline void bindMatrixBuffer(GLuint bindingPoint) const {
      matrixBuffer.bindBase(gl::StorageBuffer::Target::UNIFORM, bindingPoint);
    }

    virtual const Frustum& GetFrustum() const = 0;

    void CameraDebugUI() const;

  protected:
    glm::mat4 viewMatrix() const;
    virtual glm::mat4 projMatrix() const = 0;

    Matrices& buildMatrices() {
      auto view = viewMatrix();
      auto proj = projMatrix();

      auto viewProj = proj * view;

      auto invView = glm::inverse(view);
      auto invProj = glm::inverse(proj);

      auto invViewProj = glm::inverse(viewProj);

      matrices = Matrices(view, proj, viewProj, invView, invProj, invViewProj);
      return matrices;
    }

    void writeMatrices() const {
      auto mapping = matrixBuffer.getMapping();
      memcpy(mapping, &matrices, sizeof(Matrices));
    }

    Rotation rotation;
    glm::vec3 position;
    Matrices matrices;
    gl::StorageBuffer matrixBuffer;
    float delta = 0.0;
  };

  class PerspectiveCamera : public Camera {
  public:
    PerspectiveCamera(float nearPlane, float farPlane, float aspectRatio,
                      float fov);
    ~PerspectiveCamera() = default;

    void update(const Input& input, float dt) override;

    const Frustum& GetFrustum() const override { return m_frustum; };

    glm::mat4 projMatrix() const override { return m_projMatrix; }

  protected:
    glm::mat4 m_projMatrix;
    Frustum m_frustum;
  };
} // namespace engine
