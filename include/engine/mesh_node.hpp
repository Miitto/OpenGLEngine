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
      if (mesh->getFrameCount() > 0) {
        frameTime -= info.frameDelta;
        while (frameTime < 0.0f) {
          frameTime += mesh->getOneOverFrameRate();
          currentFrame = (currentFrame + 1) % mesh->getFrameCount();
        }
      }

      engine::scene::Node::update(info);
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
                       mesh->getJointCount(), baseVertex);

      glUniform4uiv(0, 1, &uInfo.x);
      glUniform1ui(1, currentFrame);

      glDispatchCompute(mesh->GetVertexCount(), 1, 1);

      baseVertex += mesh->GetVertexCount();

      engine::scene::Node::skinVertices(baseVertex);
    }

    virtual void writeInstanceData(gl::MappingRef& mapping, GLuint& instances,
                                   gl::MappingRef& textureMapping) override {
      auto modelMatrix = getModelMatrix();
      mapping.write(&modelMatrix, sizeof(glm::mat4));
      mapping += sizeof(glm::mat4);

      baseInstance = instances;
      instances += 1;

      mesh->writeTextureSets(textureMapping);

      engine::scene::Node::writeInstanceData(mapping, instances,
                                             textureMapping);
    }

    virtual void writeBatchedDraws(gl::MappingRef& mapping,
                                   GLuint& writtenDraws) const override {
      auto written =
          mesh->writeBatchedDraws(mapping, baseVertex, 1, baseInstance);
      writtenDraws += written;

      engine::scene::Node::writeBatchedDraws(mapping, writtenDraws);
    }

    void setFrame(uint32_t newFrame) { currentFrame = newFrame; }

  protected:
    std::shared_ptr<engine::mesh::Mesh> mesh;

    uint32_t baseVertex = 0;
    float frameTime = 0.0f;
    uint32_t currentFrame = 0;
    uint32_t baseInstance = 0;
  };
} // namespace engine::scene