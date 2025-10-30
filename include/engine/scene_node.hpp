#pragma once

#include "frame_info.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace engine {
  class Camera;
} // namespace engine

namespace engine {
  namespace scene {
    class Node {

      enum FlagBits {
        TRANSPARENT = 1 << 0,
        DRAWABLE = 1 << 1,
      };

    public:
      struct Transforms {
        glm::mat4 local = {1.0};
        glm::mat4 world = {1.0};
      };

      Node(bool transparent, bool shouldDraw);
      virtual ~Node() = default;

      inline void SetTransform(const glm::mat4& matrix) {
        m_transforms.local = matrix;
        UpdateTransforms();
      }
      inline const Transforms& GetTransforms() const { return m_transforms; }

      inline void SetScale(const glm::vec3& scale) { m_scale = scale; }

      void SetParent(Node* parent) {
        m_parent = parent;
        m_transforms.world = m_parent->m_transforms.world * m_transforms.local;
      }
      inline bool HasParent() const { return m_parent != nullptr; }
      inline const Node& GetParent() const { return *m_parent; }
      inline Node& GetParent() { return *m_parent; }
      inline const glm::vec3& GetScale() const { return m_scale; }
      inline bool isTransparent() const { return (flags & TRANSPARENT) != 0; }
      inline bool shouldDraw() const { return (flags & DRAWABLE) != 0; }

      void AddChild(const std::shared_ptr<Node>& child);
      void UpdateBoundingRadius();

      virtual void update(const engine::FrameInfo& info);
      virtual void render(const engine::FrameInfo& info,
                          const engine::Camera& camera);

      inline float GetBoundingRadius() const { return m_absBoundingRadius; }
      inline void SetBoundingRadius(float radius) {
        m_boundingRadius = radius;
        if (m_boundingRadius > m_absBoundingRadius) {
          m_absBoundingRadius = m_boundingRadius;
        }
      }

      std::vector<std::shared_ptr<Node>>& GetChildren() { return m_children; }
      std::vector<std::shared_ptr<Node>>::iterator begin() {
        return m_children.begin();
      }
      std::vector<std::shared_ptr<Node>>::iterator end() {
        return m_children.end();
      }
      std::vector<std::shared_ptr<Node>>::const_iterator begin() const {
        return m_children.begin();
      }
      std::vector<std::shared_ptr<Node>>::const_iterator end() const {
        return m_children.end();
      }
      std::vector<std::shared_ptr<Node>>::const_iterator cbegin() const {
        return m_children.cbegin();
      }
      std::vector<std::shared_ptr<Node>>::const_iterator cend() const {
        return m_children.cend();
      }

    protected:
      void UpdateTransforms();

      Node* m_parent = nullptr;
      char flags = 0;
      Transforms m_transforms = {};
      glm::vec3 m_scale = {};
      std::vector<std::shared_ptr<Node>> m_children = {};

      float m_boundingRadius = 1.0f;
      float m_absBoundingRadius = 1.0f;
    };
  } // namespace scene
} // namespace engine
