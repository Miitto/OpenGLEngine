#pragma once

#include "engine/mesh/mesh.hpp"
#include "engine/scene_node.hpp"
#include <memory>

namespace engine::scene {
  class MeshNode : public engine::scene::Node {
  public:
    MeshNode() = delete;

    MeshNode(const std::shared_ptr<engine::mesh::Mesh>& mesh)
        : engine::scene::Node(engine::scene::Node::RenderType::LIT, true),
          mesh(mesh) {}

    virtual ~MeshNode() = default;

    void update(const engine::FrameInfo& info) override {
      if (mesh->getFrameCount() > 0)
        frame += info.frameDelta * mesh->getOneOverFrameRate();

      engine::scene::Node::update(info);
    }
    void render(const engine::FrameInfo& info, const engine::Camera& camera,
                const engine::Frustum& frustum) override {
      engine::scene::Node::render(info, camera, frustum);
    }

    virtual Node::DrawParams getBatchDrawParams() const override {
      DrawParams params = {
          .instances = 1,
          .maxIndirectCmds = mesh->GetSubMeshCount(),
          .maxVertices = mesh->GetVertexCount(),
      };
      return params + engine::scene::Node::getBatchDrawParams();
    }

    virtual void skinVertices(uint32_t& baseVertex) {
      this->baseVertex = baseVertex;

      glm::uvec4 uInfo(mesh->getVertexOffset(), mesh->getStartJointIndex(),
                       mesh->getFrameCount(), baseVertex);

      glUniform4uiv(0, 1, &uInfo.x);
      glUniform1f(1, frame);

      glDispatchCompute(mesh->GetVertexCount(), 1, 1);

      engine::scene::Node::skinVertices(baseVertex);
    }

    virtual void writeInstanceData(gl::MappingRef& mapping) const override {
      auto modelMatrix = getModelMatrix();
      mapping.write(&modelMatrix, sizeof(glm::mat4));
      mapping += sizeof(glm::mat4);

      engine::scene::Node::writeInstanceData(mapping);
    }

    virtual void writeBatchedDraws(gl::MappingRef& mapping,
                                   GLuint& writtenDraws) const override {
      writtenDraws += mesh->writeBatchedDraws(mapping, baseVertex, 1, 0);

      engine::scene::Node::writeBatchedDraws(mapping, writtenDraws);
    }

    void setFrame(float newFrame) { frame = newFrame; }

  protected:
    std::shared_ptr<engine::mesh::Mesh> mesh;

    uint32_t baseVertex = 0;
    float frame = 0.0f;
  };
} // namespace engine::scene