#pragma once
#include "engine/frustum.hpp"
#include "engine/input.hpp"
#include <gl/buffer.hpp>
#include <glm/glm.hpp>
#include <glm\gtc\quaternion.hpp>

namespace engine {
  /// <summary>
  /// Generic camera class.
  /// Handles position, rotation, and view matrix calculation and manipulation.
  /// Expects derived classes to handle projection matrix calculation.
  /// </summary>
  class Camera {
  public:
    using Rotation = glm::vec2;

    /// <summary>
    /// Matrices struct sent to the GPU.
    /// </summary>
    struct Matrices {
      glm::mat4 view = glm::mat4(1.0);
      glm::mat4 proj = glm::mat4(1.0);
      glm::mat4 viewProj = glm::mat4(1.0);
      glm::mat4 invView = glm::mat4(1.0);
      glm::mat4 invProj = glm::mat4(1.0);
      glm::mat4 invViewProj = glm::mat4(1.0);
      glm::vec2 resolution = glm::vec2(1.0);
      glm::vec2 uvRange = glm::vec2(0.0, 1.0);
    };

    /// <summary>
    /// Creates a camera at the origin pointing forwards.
    /// </summary>
    Camera();
    /// <summary>
    /// Creates a camera with the given rotation and position.
    /// </summary>
    /// <param name="rotation"></param>
    /// <param name="position"></param>
    Camera(Rotation rotation, const glm::vec3& position);
    ~Camera() = default;

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) noexcept = default;
    Camera& operator=(Camera&&) = default;

    /// <summary>
    /// Updates the camera based on user input relative to the delta time.
    /// Will update the view matrix and write it to the matrix buffer.
    /// Should be called once per frame, as soon as possible in update.
    /// </summary>
    /// <param name="input">User input data</param>
    /// <param name="dt">Delta time</param>
    virtual void update(const Input& input, float dt, bool acceptInput = true);
    /// <summary>
    /// Should be called whenever the viewport is resized.
    /// </summary>
    /// <param name="width">New viewport width</param>
    /// <param name="height">New viewport height</param>
    virtual void onResize(int width, int height,
                          glm::vec2 uvRange = glm::vec2(0.0, 1.0));

    inline const Matrices& GetMatrices() const { return matrices; }
    inline const glm::vec3& GetPosition() const { return position; }
    inline void SetPosition(const glm::vec3& pos) { position = pos; }
    inline const glm::quat& GetRotation() const { return rotation; }
    inline void SetRotation(const glm::quat& rot) { rotation = rot; }

    inline void EnableMouse(bool enable) { enableMouse = enable; }

    /// <summary>
    /// Returns the camera's forward vector based on its current rotation.
    /// </summary>
    /// <returns>Camera's forward vector</returns>
    inline glm::vec3 forward() const {
      return glm::normalize(rotation * glm::vec3(0.0f, 0.0f, -1.0f));
    }

    /// <summary>
    /// Binds the camera's matrix UBO to the given binding point.
    /// </summary>
    /// <param name="bindingPoint">Index to bind the buffer to</param>
    inline void bindMatrixBuffer(GLuint bindingPoint) const {
      matrixBuffer.bindBase(gl::Buffer::StorageTarget::UNIFORM, bindingPoint);
    }

    /// <summary>
    /// Gets the camera's frustum.
    /// </summary>
    /// <returns>Current camera frustum</returns>
    virtual const Frustum& GetFrustum() const = 0;

    /// <summary>
    /// Renderes the camera debug UI.
    /// Expects an active ImGui frame, as it does not create its own.
    /// </summary>
    void CameraDebugUI();

  protected:
    /// <summary>
    /// Calculates the view matrix based on the current position and rotation.
    /// </summary>
    /// <returns>Camera's view matrix</returns>
    glm::mat4 viewMatrix() const;
    /// <summary>
    /// Returns the projection matrix.
    /// It is recommended for derived classes to store the perspective matrix as
    /// it does not need to be recalculated often.
    /// </summary>
    /// <returns>Camera's perspective matrix</returns>
    virtual glm::mat4 projMatrix() const = 0;

    /// <summary>
    /// Creates all matrices and stores them in the matrices member.
    /// Should be called before writing to the matrix buffer.
    /// </summary>
    /// <returns>Reference to the current camera matrices</returns>
    Matrices& buildMatrices() {
      auto view = viewMatrix();
      auto proj = projMatrix();

      auto viewProj = proj * view;

      auto invView = glm::inverse(view);
      auto invProj = glm::inverse(proj);

      auto invViewProj = glm::inverse(viewProj);

      matrices = Matrices(view, proj, viewProj, invView, invProj, invViewProj,
                          matrices.resolution, matrices.uvRange);
      return matrices;
    }

    /// <summary>
    /// Writes the current matrices to the matrix buffer.
    /// Does not update the matrices member, buildMatrices should be called
    /// first if required.
    /// </summary>
    inline void writeMatrices() const {
      matrixMapping.write(&matrices, sizeof(Matrices));
    }

    glm::quat rotation;
    glm::vec3 position;
    Matrices matrices;
    gl::Buffer matrixBuffer;
    gl::Mapping matrixMapping;
    float delta = 0.0;
    int polygonType = 0;
    bool vsync = true;
    bool enableMouse = true;
    uint32_t id;
  };

  /// <summary>
  /// Camera with a perspective projection matrix.
  /// </summary>
  class PerspectiveCamera : public Camera {
  public:
    /// <summary>
    /// Creates a new perspective camera.
    /// </summary>
    /// <param name="nearPlane">Near clip distance</param>
    /// <param name="farPlane">Far clip distance</param>
    /// <param name="aspectRatio">Viewport aspect ratio</param>
    /// <param name="fov">FOV in radians</param>
    PerspectiveCamera(float nearPlane, float farPlane, float aspectRatio,
                      float fov);
    ~PerspectiveCamera() = default;
    PerspectiveCamera(const PerspectiveCamera&) = delete;
    PerspectiveCamera& operator=(const PerspectiveCamera&) = delete;
    PerspectiveCamera(PerspectiveCamera&&) noexcept = default;
    PerspectiveCamera& operator=(PerspectiveCamera&&) = default;

    /// <summary>
    /// Camera update function. Delegates to base Camera update and updates the
    /// frustum.
    /// </summary>
    /// <param name="input">User input data</param>
    /// <param name="dt">Delta time</param>
    void update(const Input& input, float dt, bool acceptInput = true) override;
    /// <summary>
    /// Handles viewport resize. Updates the projection matrix and frustum.
    /// </summary>
    /// <param name="width">New viewport width</param>
    /// <param name="height">New viewport height</param>
    void onResize(int width, int height,
                  glm::vec2 uvRange = glm::vec2(0.0, 1.0)) override;

    const Frustum& GetFrustum() const override { return m_frustum; };

    /// <summary>
    /// Returns the cached projection matrix.
    /// </summary>
    /// <returns>Camera's projection matrix</returns>
    glm::mat4 projMatrix() const override { return matrices.proj; }

    inline const float getNear() const { return near; }
    inline const float getFar() const { return far; }
    inline const float getFov() const { return fov; }

  protected:
    float fov;
    float near;
    float far;

    Frustum m_frustum;
  };
} // namespace engine
