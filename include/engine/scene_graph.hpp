#pragma once

#include "camera.hpp"
#include "frame_info.hpp"
#include "scene_node.hpp"

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

        std::vector<Pair> opaque;
        std::vector<Pair> transparent;

        inline void render(const engine::FrameInfo& info,
                           const engine::Camera& camera) const {
          for (const auto node : opaque) {
            node.node->render(info, camera);
          }
          for (const auto node : transparent) {
            node.node->render(info, camera);
          }
        }
      };

      NodeLists BuildNodeLists(const engine::Camera& camera);

      inline void update(const engine::FrameInfo& info) {
        for (auto& root : m_roots) {
          root->update(info);
        }
      }

      inline void render(const engine::FrameInfo& info,
                         const engine::Camera& camera) {
        NodeLists lists = BuildNodeLists(camera);
        lists.render(info, camera);
      }

    protected:
      std::vector<std::shared_ptr<Node>> m_roots;
    };
  } // namespace scene
} // namespace engine
