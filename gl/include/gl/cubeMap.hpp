#pragma once

#include "gl/id.hpp"
#include <gl/texture.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace gl {
  class CubeMap {
  public:
    enum class Face : GLint {
      POSITIVE_X = 0,
      NEGATIVE_X = 1,
      POSITIVE_Y = 2,
      NEGATIVE_Y = 3,
      POSITIVE_Z = 4,
      NEGATIVE_Z = 5
    };

    inline CubeMap() { glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, _id); }
    inline ~CubeMap() {
      if (_id != 0)
        glDeleteTextures(1, _id);
    }
    CubeMap(const CubeMap&) = delete;
    CubeMap& operator=(const CubeMap&) = delete;
    CubeMap(CubeMap&& other) noexcept {
      if (_id != 0)
        glDeleteTextures(1, _id);
      _id = std::move(other._id);
      other._id = gl::Id(0);
    }
    CubeMap& operator=(CubeMap&& other) noexcept {
      if (this != &other) {
        if (_id != 0)
          glDeleteTextures(1, _id);

        _id = std::move(other._id);
        other._id = 0;
      }
      return *this;
    }

    const gl::Id& id() const { return _id; }

    inline void storage(GLsizei levels, GLenum internalFormat,
                        const glm::uvec2& size) {
      glTextureStorage2D(_id, levels, internalFormat, size.x, size.y);
    }
    inline void subImage(GLuint level, GLint xOffset, GLint yOffset, Face face,
                         GLsizei width, GLsizei height, GLsizei faces,
                         GLenum format, GLenum type, const void* data) {
      glTextureSubImage3D(_id, level, xOffset, yOffset,
                          static_cast<GLint>(face), width, height, faces,
                          format, type, data);
    }

    inline void setParameter(GLenum pname, GLint param) {
      glTextureParameteri(_id, pname, param);
    }

    inline void generateMipmaps() const { glGenerateTextureMipmap(_id); }

    inline void bind(GLuint unit) const { glBindTextureUnit(unit, _id); }

    inline const gl::TextureHandle& createHandle() {
      RawTextureHandle rawHandle = glGetTextureHandleARB(_id);
      _handle = TextureHandle(rawHandle);
      return _handle;
    }

    inline const gl::TextureHandle& handle() const { return _handle; }

  protected:
    gl::Id _id;
    gl::TextureHandle _handle = 0;
  };
} // namespace gl