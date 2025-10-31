#pragma once

#include <gl/id.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace gl {
  class Texture {
  public:
    struct Size {
      GLsizei width;
      GLsizei height;
    };

  private:
    gl::Id m_id = 0;
    gl::Texture::Size m_size{};

  public:
    Texture() { glCreateTextures(GL_TEXTURE_2D, 1, m_id); }
    Texture(Size size, GLenum format, GLenum internalFormat, void* data) {
      glCreateTextures(GL_TEXTURE_2D, 1, m_id);

      storage(1, internalFormat, size);
      subImage(0, 0, 0, size.width, size.height, format, GL_UNSIGNED_BYTE,
               data);
    }
    const gl::Id& id() const { return m_id; }

    inline void label(const char* name) const {
      glObjectLabel(GL_TEXTURE, m_id, -1, name);
    }

    inline void generateMipmap() const { glGenerateTextureMipmap(m_id); }

    constexpr inline static GLenum formatFromChannels(int channels) {
      switch (channels) {
      case 1:
        return GL_RED;
      case 2:
        return GL_RG;
      case 3:
        return GL_RGB;
      case 4:
        return GL_RGBA;
      default:
        return GL_INVALID_ENUM;
      }
    }

    constexpr inline static GLenum internalFormatFromChannels(int channels) {
      switch (channels) {
      case 1:
        return GL_R8;
      case 2:
        return GL_RG8;
      case 3:
        return GL_RGB8;
      case 4:
        return GL_RGBA8;
      default:
        return GL_INVALID_ENUM;
      }
    }

    void bind(uint8_t unit) const { glBindTextureUnit(unit, m_id); }
    static void unbind(GLenum unit) { glBindTextureUnit(unit, 0); }
    void setParameter(GLenum pname, GLint param) const {
      glTextureParameteri(m_id, pname, param);
    }
    void storage(GLint level, GLenum internalformat, gl::Texture::Size size) {
      m_size = size;
      glTextureStorage2D(m_id, level, internalformat, size.width, size.height);
    }
    void subImage(GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                  GLsizei height, GLenum format, GLenum type,
                  const void* pixels) const {
      glTextureSubImage2D(m_id, level, xoffset, yoffset, width, height, format,
                          type, pixels);
    }

    const Size& size() const { return m_size; }
  };

  class Texture2DMultiSample {
  private:
    gl::Id m_id = 0;
    gl::Texture::Size m_size{};

  public:
    Texture2DMultiSample() {
      glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, m_id);
    }
    const gl::Id& id() const { return m_id; }

    void bind(GLenum unit) const { glBindTextureUnit(unit, m_id); }
    static void unbind(GLenum unit) { glBindTextureUnit(unit, 0); }
    void setParameter(GLenum pname, GLint param) const {
      glTextureParameteri(m_id, pname, param);
    }
    void storage(GLint samples, GLenum internalformat, gl::Texture::Size size) {
      m_size = size;
      glTextureStorage2DMultisample(m_id, samples, internalformat, size.width,
                                    size.height, GL_FALSE);
    }

    const gl::Texture::Size& size() const { return m_size; }
  };
} // namespace gl