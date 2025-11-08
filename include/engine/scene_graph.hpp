#pragma once

#include "camera.hpp"
#include "engine/scene_node.hpp"
#include "frame_info.hpp"
#include <engine/frustum.hpp>

namespace engine {
  namespace scene {
    class Graph {
    public:
      Graph() = default;
      ~Graph() = default;

      // Non-copyable
      Graph(const Graph&) = delete;
      Graph& operator=(const Graph&) = delete;

      // Update parent pointers after move. Lets Graph & root sit on stack.
      // Maybe a minor performance benefit?
      inline Graph(Graph&& o) noexcept {
        m_roots = std::move(o.m_roots);
        for (auto& root : m_roots) {
          for (auto& child : root->GetChildren()) {
            child->SetParent(root.get());
          }
        }
      }
      inline Graph& operator=(Graph&& o) noexcept {
        if (this != &o) {
          m_roots = std::move(o.m_roots);
          for (auto& root : m_roots) {
            for (auto& child : root->GetChildren()) {
              child->SetParent(root.get());
            }
          }
        }
        return *this;
      }

      inline void AddChild(const std::shared_ptr<Node>& child) {
        m_roots.emplace_back(child);
      }

      struct NodeLists {
        struct Pair {
          Node* node;
          float dist;
        };

        std::vector<Pair> lit;
        std::vector<Pair> opaque;
        std::vector<Pair> transparent;

        inline void renderLit(const engine::Frustum& frustum) const {
          for (const auto node : lit) {
            node.node->render(frustum);
          }
        }

        inline void renderLitDepthOnly() const {
          for (const auto node : lit) {
            node.node->renderDepthOnly();
          }
        }

        inline void renderOpaque(const engine::Frustum& frustum) const {
          for (const auto node : opaque) {
            node.node->render(frustum);
          }
        }

        inline void renderOpaqueDepthOnly() const {
          for (const auto node : opaque) {
            node.node->renderDepthOnly();
          }
        }

        inline void renderTransparent(const engine::Frustum& frustum) const {
          for (const auto node : transparent) {
            node.node->render(frustum);
          }
        }
      };

      NodeLists BuildNodeLists(const engine::Frustum& frustum,
                               const glm::vec3& position);

      inline void update(const engine::FrameInfo& info) {
        for (auto& root : m_roots) {
          root->update(info);
        }
      }

      inline const std::vector<std::shared_ptr<Node>>& GetRoots() const {
        return m_roots;
      }

    protected:
      std::vector<std::shared_ptr<Node>> m_roots;
    };
  } // namespace scene
} // namespace engine
