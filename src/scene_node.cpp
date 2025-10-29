#include "engine/scene_node.hpp"

namespace engine::scene {
  Node::Node(bool transparent, bool shouldDraw) {
    if (transparent) {
      flags |= TRANSPARENT;
    }
    if (shouldDraw) {
      flags |= DRAWABLE;
    }
  }

  void Node::AddChild(const std::shared_ptr<Node>& child) {
    m_children.emplace_back(child);
    child->m_parent = this;
    child->UpdateBoundingRadius();
  }

  void Node::update(float dt) {
    if (m_parent) {
      m_transforms.world = m_parent->m_transforms.world * m_transforms.local;
    } else {
      m_transforms.world = m_transforms.local;
    }

    for (auto& child : m_children) {
      child->update(dt);
    }
  }

  void Node::render(const engine::Camera& camera) {
    for (auto& child : *this) {
      child->render(camera);
    }
  }

  void Node::UpdateBoundingRadius() {
    if (!m_parent)
      return;
    glm::vec3 relPos(m_transforms.local[3]);
    float adjBoundRad = relPos.length() + m_boundingRadius;
    m_parent->m_absBoundingRadius =
        std::max(m_parent->m_boundingRadius, adjBoundRad);
    m_parent->UpdateBoundingRadius();
  }

  void Node::UpdateTransforms() {
    if (m_parent) {
      m_transforms.world = m_parent->m_transforms.world * m_transforms.local;
    }
    for (auto& child : m_children) {
      child->UpdateTransforms();
    }
  }

} // namespace engine::scene