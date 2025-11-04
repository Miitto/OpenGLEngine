#include "engine/scene_node.hpp"
#include <engine\frustum.hpp>
#include <glm\ext\matrix_transform.hpp>

namespace engine::scene {
  Node::Node(RenderType renderType, bool shouldDraw) {
    switch (renderType) {
    case RenderType::LIT:
      flags |= LIT;
      break;
    case RenderType::TRANSPARENT:
      flags |= TRANSPARENT;
      break;
    }

    if (shouldDraw) {
      flags |= DRAWABLE;
    }
  }

  Node::Node(Node&& o) noexcept
      : m_parent(o.m_parent), flags(o.flags), m_transforms(o.m_transforms),
        m_scale(o.m_scale), m_children(std::move(o.m_children)),
        m_boundingRadius(o.m_boundingRadius),
        m_absBoundingRadius(o.m_absBoundingRadius) {
    for (auto& m_children : m_children) {
      m_children->m_parent = this;
    }
  }

  Node& Node::operator=(Node&& o) noexcept {
    if (this != &o) {
      m_parent = o.m_parent;
      flags = o.flags;
      m_transforms = o.m_transforms;
      m_scale = o.m_scale;
      m_children = std::move(o.m_children);
      m_boundingRadius = o.m_boundingRadius;
      m_absBoundingRadius = o.m_absBoundingRadius;
      for (auto& m_children : m_children) {
        m_children->m_parent = this;
      }
    }

    return *this;
  }

  void Node::AddChild(const std::shared_ptr<Node>& child) {
    m_children.emplace_back(child);
    child->m_parent = this;
    child->UpdateBoundingRadius();
  }

  bool Node::shouldRender(const engine::Frustum& frustum) const {
    return shouldDraw();
  }

  void Node::update(const engine::FrameInfo& info) {
    if (m_parent) {
      m_transforms.world = m_parent->m_transforms.world * m_transforms.local;
    } else {
      m_transforms.world = m_transforms.local;
    }

    for (auto& child : m_children) {
      child->update(info);
    }
  }

  void Node::render(const engine::FrameInfo& info, const engine::Camera& camera,
                    const engine::Frustum& frustum) {
    for (auto& child : *this) {
      if (child->shouldRender(frustum))
        child->render(info, camera, frustum);
    }
  }

  glm::mat4 Node::getModelMatrix() const {
    return glm::scale(m_transforms.world, GetScale());
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