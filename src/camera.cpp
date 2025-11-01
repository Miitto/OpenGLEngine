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

  Camera::Camera()
      : rotation({0.0, 0.0}), position({}), matrices(Matrices()),
        matrixBuffer({sizeof(Matrices), nullptr,
                      gl::Buffer::Usage::DYNAMIC | gl::Buffer::Usage::WRITE |
                          gl::Buffer::Usage::PERSISTENT |
                          gl::Buffer::Usage::COHERENT}) {
    matrixBuffer.map(gl::Buffer::Mapping::COHERENT |
                     gl::Buffer::Mapping::PERSISTENT |
                     gl::Buffer::Mapping::WRITE);
  }

  Camera::Camera(Rotation rotation, const glm::vec3& position)
      : rotation(rotation), position(position), matrices(Matrices()),
        matrixBuffer({sizeof(Matrices), nullptr,
                      gl::Buffer::Usage::DYNAMIC | gl::Buffer::Usage::WRITE |
                          gl::Buffer::Usage::PERSISTENT |
                          gl::Buffer::Usage::COHERENT}) {
    matrixBuffer.map(gl::Buffer::Mapping::COHERENT |
                     gl::Buffer::Mapping::PERSISTENT |
                     gl::Buffer::Mapping::WRITE);
  }

  PerspectiveCamera::PerspectiveCamera(float nearClip, float farClip,
                                       float aspect, float fov)
      : Camera(), fov(fov), near(nearClip), far(farClip),
        m_frustum(matrices.viewProj) {
    matrices.proj = glm::perspective(fov, aspect, nearClip, farClip);
    buildMatrices();
    writeMatrices();
  }

  void Camera::update(const Input& input, float dt) {
    delta = dt;
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

    buildMatrices();
    writeMatrices();
  }

  void PerspectiveCamera::update(const Input& input, float dt) {
    Camera::update(input, dt);
    m_frustum = Frustum(matrices.viewProj);
  }

  void PerspectiveCamera::onResize(int width, int height) {
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    matrices.proj = glm::perspective(fov, aspect, near, far);
    buildMatrices();
    writeMatrices();
    m_frustum = Frustum(matrices.viewProj);
  }

  glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(position, position + forward(), UP);
  }

  void Camera::CameraDebugUI() {
    ImGui::Text("Delta Time: %.4f s (%.2f FPS)", delta, 1.0f / delta);
    ImGui::Text("Position: (%.2f, %.2f, %.2f)", position.x, position.y,
                position.z);
    ImGui::Text("Rotation: (%.2f, %.2f)", rotation.pitch, rotation.yaw);

    bool needSetPolygonMode = false;

    const char* polygonTypes[] = {"Fill", "Wireframe", "Point"};

    if (ImGui::BeginCombo("Polygon Type", polygonTypes[polygonType])) {
      if (ImGui::Selectable("Fill", polygonType == 0)) {
        needSetPolygonMode = true;
        polygonType = 0;
      }
      if (ImGui::Selectable("Wireframe", polygonType == 1)) {
        needSetPolygonMode = true;
        polygonType = 1;
      }
      if (ImGui::Selectable("Point", polygonType == 2)) {
        needSetPolygonMode = true;
        polygonType = 2;
      }

      ImGui::EndCombo();
    }

    if (ImGui::Checkbox("VSync", &vsync)) {
      int interval = vsync ? 1 : 0;
      glfwSwapInterval(interval);
    }

    if (needSetPolygonMode) {
      switch (polygonType) {
      case 0:
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
      case 1:
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
      case 2:
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        break;
      default:
        break;
      }
    }
  }
} // namespace engine
