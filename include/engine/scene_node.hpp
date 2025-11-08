#pragma once

#include "frame_info.hpp"
#include <engine/frustum.hpp>
#include <gl/buffer.hpp>
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
        LIT = 1 << 2,
      };

    public:
      enum class RenderType {
        OPAQUE,
        TRANSPARENT,
        LIT,
      };

      struct Transforms {
        glm::mat4 local = {1.0};
        glm::mat4 world = {1.0};
      };

      Node(RenderType renderType, bool shouldDraw);
      virtual ~Node() = default;

      Node(const Node&) = delete;
      Node& operator=(const Node&) = delete;
      Node(Node&&) noexcept;
      Node& operator=(Node&&) noexcept;

#pragma region Get Set
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

      inline RenderType getRenderType() const {
        if ((flags & LIT) != 0) {
          return RenderType::LIT;
        } else if ((flags & TRANSPARENT) != 0) {
          return RenderType::TRANSPARENT;
        } else {
          return RenderType::OPAQUE;
        }
      }

      glm::mat4 getModelMatrix() const;

#pragma endregion

      void AddChild(const std::shared_ptr<Node>& child);
      void UpdateBoundingRadius();

      virtual void update(const engine::FrameInfo& info);
      virtual void render(const engine::Frustum& frustum);
      virtual void renderDepthOnly();

      inline float GetBoundingRadius() const { return m_absBoundingRadius; }
      inline void SetBoundingRadius(float radius) {
        m_boundingRadius = radius;
        if (m_boundingRadius > m_absBoundingRadius) {
          m_absBoundingRadius = m_boundingRadius;
        }
      }

      bool shouldDraw() const { return (flags & DRAWABLE) != 0; }
      virtual bool shouldRender(const engine::Frustum& frustum) const;

      struct DrawParams {
        GLuint instances;
        GLuint maxIndirectCmds;
        GLuint maxVertices;

        DrawParams& operator+=(const DrawParams& o) {
          instances += o.instances;
          maxIndirectCmds += o.maxIndirectCmds;
          maxVertices += o.maxVertices;
          return *this;
        }

        DrawParams operator+(const DrawParams& o) const {
          return {.instances = instances + o.instances,
                  .maxIndirectCmds = maxIndirectCmds + o.maxIndirectCmds,
                  .maxVertices = maxVertices + o.maxVertices};
        }
      };

      virtual DrawParams getBatchDrawParams() const {
        DrawParams maxDraws = {0, 0, 0};
        for (const auto& child : m_children) {
          maxDraws += child->getBatchDrawParams();
        }

        return maxDraws;
      }

      virtual void skinVertices(uint32_t& baseVertex) {
        for (const auto& child : m_children) {
          child->skinVertices(baseVertex);
        }
      }

      virtual void writeInstanceData(gl::MappingRef& mapping, GLuint& instances,
                                     gl::MappingRef& textureMapping) {
        for (const auto& child : m_children) {
          child->writeInstanceData(mapping, instances, textureMapping);
        }
      }

      virtual void writeBatchedDraws(gl::MappingRef& mapping,
                                     GLuint& writtenDraws) const {
        for (const auto& child : m_children) {
          child->writeBatchedDraws(mapping, writtenDraws);
        }
      }

#pragma region Children Iterators
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
#pragma endregion

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
