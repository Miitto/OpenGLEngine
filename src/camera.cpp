#include "engine/camera.hpp"

#include <glm/glm.hpp>
#include <glm\ext\matrix_clip_space.hpp>
#include <glm\ext\matrix_transform.hpp>

#include "GLFW/glfw3.h"
#include "engine/constants.hpp"
#include "imgui/imgui.h"
#include "logger.hpp"

namespace {
  uint32_t camId = 0;
}

namespace engine {
  const float MOVE_SPEED = 15.0f;
  const float FAST_MOVE_SPEED = 150.f;

  Camera::Camera()
      : rotation(glm::quat(glm::vec3(0, 0, 0))), position({}),
        matrices(Matrices()),
        matrixBuffer({sizeof(Matrices), nullptr,
                      gl::Buffer::Usage::DYNAMIC | gl::Buffer::Usage::WRITE |
                          gl::Buffer::Usage::PERSISTENT |
                          gl::Buffer::Usage::COHERENT}),
        id(camId++) {
    matrixMapping = matrixBuffer.map(gl::Buffer::Mapping::COHERENT |
                                     gl::Buffer::Mapping::PERSISTENT |
                                     gl::Buffer::Mapping::WRITE);
  }

  Camera::Camera(Rotation rotation, const glm::vec3& position)
      : position(position), matrices(Matrices()),
        matrixBuffer({sizeof(Matrices), nullptr,
                      gl::Buffer::Usage::DYNAMIC | gl::Buffer::Usage::WRITE |
                          gl::Buffer::Usage::PERSISTENT |
                          gl::Buffer::Usage::COHERENT}),
        id(camId++) {
    glm::vec3 radians = glm::radians(glm::vec3(rotation.x, rotation.y, 0.0f));
    glm::quat pitchQuat =
        glm::angleAxis(radians.x, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat yawQuat = glm::angleAxis(radians.y, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat rollQuat = glm::angleAxis(radians.z, glm::vec3(0.0f, 0.0f, 1.0f));

    this->rotation = glm::normalize(yawQuat * pitchQuat * rollQuat);

    matrixMapping = matrixBuffer.map(gl::Buffer::Mapping::COHERENT |
                                     gl::Buffer::Mapping::PERSISTENT |
                                     gl::Buffer::Mapping::WRITE);
  }

  void Camera::onResize(int width, int height, glm::vec2 uvRange) {
    glm::vec2 size =
        glm::vec2(static_cast<float>(width), static_cast<float>(height));
    matrices.resolution = size;
    matrices.uvRange = uvRange;
    matrixMapping.write(&matrices.resolution, sizeof(glm::vec2) * 2,
                        offsetof(Matrices, resolution));
  }

  PerspectiveCamera::PerspectiveCamera(float nearClip, float farClip,
                                       float aspect, float fov)
      : Camera(), fov(fov), near(nearClip), far(farClip),
        m_frustum(matrices.viewProj) {
    matrices.proj = glm::perspective(fov, aspect, farClip, nearClip);
    buildMatrices();
    writeMatrices();
  }

  void Camera::update(const Input& input, float dt, bool acceptInput) {
    delta = dt;
    if (acceptInput) {
      glm::vec3 eulerRot = glm::eulerAngles(rotation);

      float rollFactor = 1.f;
      // Clamp roll
      if (eulerRot.z > glm::radians(90.f) || eulerRot.z < glm::radians(-90.f)) {
        eulerRot.z = glm::radians(180.f);
        rollFactor = -1.f;
      } else {
        eulerRot.z = 0.f;
      }

      if (enableMouse) {
        auto& delta = input.mouse().delta;

        auto eulerDelta = (glm::vec2(delta.y, delta.x) * 0.5f);
        glm::vec3 radians =
            glm::radians(glm::vec3(eulerDelta.x, eulerDelta.y, 0.0f));

        eulerRot.x -= radians.x;
        eulerRot.y -= radians.y * rollFactor;
      }

      if (input.isKeyDown(GLFW_KEY_UP))
        eulerRot.x += glm::radians(100.0f * dt);
      if (input.isKeyDown(GLFW_KEY_DOWN))
        eulerRot.x -= glm::radians(100.0f * dt);
      if (input.isKeyDown(GLFW_KEY_LEFT))
        eulerRot.y += glm::radians(100.0f * dt) * rollFactor;
      if (input.isKeyDown(GLFW_KEY_RIGHT))
        eulerRot.y -= glm::radians(100.0f * dt) * rollFactor;

      // Clamp pitch
      if (rollFactor == 1.f)
        eulerRot.x =
            glm::clamp(eulerRot.x, glm::radians(-89.0f), glm::radians(89.0f));
      else {
        if (eulerRot.x < glm::radians(91.f) && eulerRot.x > glm::radians(0.f))
          eulerRot.x = glm::radians(91.f);
        else if (eulerRot.x > glm::radians(-91.f) &&
                 eulerRot.x < glm::radians(0.f))
          eulerRot.x = glm::radians(-91.f);
      }

      rotation = glm::quat(eulerRot);

      auto forward = this->forward();
      auto right = glm::normalize(glm::cross(forward, UP));
      auto speed = MOVE_SPEED;

      auto check = [&](int key, const glm::vec3& dir) {
        if (input.isKeyDown(key)) {
          position += dir * dt * speed;
        }
      };

      if (input.isKeyDown(GLFW_KEY_LEFT_SHIFT))
        speed = FAST_MOVE_SPEED;

      check(GLFW_KEY_W, forward);
      check(GLFW_KEY_S, forward * -1.0f);
      check(GLFW_KEY_A, right * -1.0f);
      check(GLFW_KEY_D, right);

      check(GLFW_KEY_SPACE, UP);
      check(GLFW_KEY_LEFT_CONTROL, UP * -1.0f);

      if (input.isKeyPressed(GLFW_KEY_ESCAPE)) {
        enableMouse = !enableMouse;
      }
    }
    buildMatrices();
    writeMatrices();
  }

  void PerspectiveCamera::update(const Input& input, float dt,
                                 bool acceptInput) {
    Camera::update(input, dt, acceptInput);
    m_frustum = Frustum(matrices.viewProj);
  }

  void PerspectiveCamera::onResize(int width, int height, glm::vec2 uvRange) {
    Camera::onResize(width, height, uvRange);
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    matrices.proj = glm::perspective(fov, aspect, far, near);
    buildMatrices();
    writeMatrices();
    m_frustum = Frustum(matrices.viewProj);
  }

  glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(position, position + forward(),
                       glm::vec3(0.0f, 1.0f, 0.0f));
  }

  void Camera::CameraDebugUI() {
    ImGui::PushID(id);
    ImGui::Text("Delta Time: %.4f s (%.2f FPS)", delta, 1.0f / delta);
    ImGui::Text("Position: (%.2f, %.2f, %.2f)", position.x, position.y,
                position.z);
    glm::vec3 radEuler = glm::eulerAngles(rotation);
    glm::vec3 rotation = glm::degrees(radEuler);
    ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", rotation.x, rotation.y,
                rotation.z);
    auto forward = this->forward();
    ImGui::Text("Forward: (%.2f, %.2f, %.2f)", forward.x, forward.y, forward.z);
    ImGui::Checkbox("Enable Mouse", &enableMouse);

    ImGui::PopID();
  }
} // namespace engine
