#include "engine/scene_graph.hpp"
#include "logger.hpp"
#include <algorithm>
#include <functional>

namespace engine::scene {
  Graph::NodeLists Graph::BuildNodeLists(const engine::Camera& camera) {
    NodeLists lists;

    auto& frustum = camera.GetFrustum();

    auto addNodeToList = [&](Node& node) {
      glm::vec3 nodePos(node.GetTransforms().world[3]);
      auto relCamPos = nodePos - camera.GetPosition();
      float dist = glm::dot(relCamPos, relCamPos); // Squared distance
      switch (node.getRenderType()) {
      case Node::RenderType::LIT:
        lists.lit.emplace_back(&node, dist);
        break;
      case Node::RenderType::OPAQUE:
        lists.opaque.emplace_back(&node, dist);
        break;
      case Node::RenderType::TRANSPARENT:
        lists.transparent.emplace_back(&node, dist);
        break;
      }
    };

    std::function<void(Node&)> addNodeAndChildren = [&](Node& node) {
      if (node.shouldRender(frustum)) {
        if (node.shouldDraw())
          addNodeToList(node);

        for (const auto& child : node) {
          addNodeAndChildren(*child);
        }
      }
    };

    for (auto& root : m_roots) {
      addNodeAndChildren(*root);
    }

    auto comp = [](const NodeLists::Pair& a, const NodeLists::Pair& b) {
      return a.dist < b.dist;
    };

    std::sort(lists.lit.begin(), lists.lit.end(), comp);
    std::sort(lists.opaque.begin(), lists.opaque.end(), comp);
    std::sort(lists.transparent.rbegin(), lists.transparent.rend(), comp);

    unsigned int i = 0;
    for (auto& node : lists.transparent) {
      ++i;
    }
    for (auto& node : lists.opaque) {
      ++i;
    }

    Logger::trace("Total nodes in lists: {}", i);

    return lists;
  }
} // namespace engine::scene