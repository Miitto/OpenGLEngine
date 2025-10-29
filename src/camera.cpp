#include "engine/camera.hpp"
#include <glm/glm.hpp>
#include <glm\ext\matrix_clip_space.hpp>
#include <glm\ext\matrix_transform.hpp>

#include "GLFW/glfw3.h"
#include "engine/constants.hpp"
#include "imgui/imgui.h"
#include "logger.hpp"

namespace engine {

  const float MOVE_SPEED = 30.0f;
  const float FAST_MOVE_SPEED = 100.f;

  PerspectiveCamera::PerspectiveCamera(float nearClip, float farClip,
                                       float aspect, float fov)
      : Camera(),
        m_projMatrix(glm::perspective(nearClip, farClip, aspect, fov)),
        m_frustum(m_projMatrix * BuildViewMatrix()) {}

  void Camera::update(const Input& input, float dt) {
    auto& delta = input.mouse().delta;
    rotation.pitch -= (delta.y);
    rotation.yaw -= (delta.x);

    if (input.isKeyDown(GLFW_KEY_UP))
      rotation.pitch += 100.0f * dt;
    if (input.isKeyDown(GLFW_KEY_DOWN))
      rotation.pitch -= 100.0f * dt;
    if (input.isKeyDown(GLFW_KEY_LEFT))
      rotation.yaw += 100.0f * dt;
    if (input.isKeyDown(GLFW_KEY_RIGHT))
      rotation.yaw -= 100.0f * dt;

    rotation.pitch = std::fmax(std::fmin(rotation.pitch, 89.0f), -89.0f);
    if (rotation.yaw < 0.0f)
      rotation.yaw += 360.0f;
    else if (rotation.yaw > 360.0f)
      rotation.yaw -= 360.0f;

    glm::mat4 rot = glm::rotate(glm::mat4(1.0), rotation.yaw, UP);
    glm::vec3 forward = rot * glm::vec4(FORWARD, 1.0);
    glm::vec3 right = rot * glm::vec4(RIGHT, 1.0);

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
  }

  void PerspectiveCamera::update(const Input& input, float dt) {
    Camera::update(input, dt);
    auto view = BuildViewMatrix();
    auto viewProj = m_projMatrix * view;
    m_frustum = Frustum(viewProj);
  }

  glm::mat4 Camera::BuildViewMatrix() const {
    glm::mat4 rot = glm::rotate(
        glm::rotate(glm::mat4(1.0), -rotation.pitch, RIGHT), -rotation.yaw, UP);
    glm::mat4 trans = glm::translate(glm::mat4(1.0), -position);
    return rot * trans;
  }

  void Camera::CameraDebugUI() const {
    ImGui::Text("Position: (%.2f, %.2f, %.2f)", position.x, position.y,
                position.z);
    ImGui::Text("Rotation: (%.2f, %.2f)", rotation.pitch, rotation.yaw);
  }
} // namespace engine
