#include "gl/texture.hpp"

namespace {
  inline void makeHandleResident(GLuint64 handle) {
    glMakeTextureHandleResidentARB(handle);
  }

  inline void makeHandleNonResident(GLuint64 handle) {
    glMakeTextureHandleNonResidentARB(handle);
  }

} // namespace

namespace gl {
  void TextureHandle::use() { makeHandleResident(_handle); }
  void TextureHandle::unuse() { makeHandleNonResident(_handle); }

  Texture::Texture(gl::Texture::Size size, GLenum format, GLenum internalFormat,
                   void* data)
      : Texture() {
    storage(1, internalFormat, size);
    subImage(0, 0, 0, size.width, size.height, format, GL_UNSIGNED_BYTE, data);
  }

  GLenum Texture::formatFromChannels(int channels) {
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

  GLenum Texture::internalFormatFromChannels(int channels) {
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

  void Texture::storage(GLint level, GLenum internalformat,
                        gl::Texture::Size size) {
    m_size = size;
    glTextureStorage2D(m_id, level, internalformat, size.width, size.height);
  }

  void Texture::subImage(GLint level, GLint xoffset, GLint yoffset,
                         GLsizei width, GLsizei height, GLenum format,
                         GLenum type, const void* pixels) const {
    glTextureSubImage2D(m_id, level, xoffset, yoffset, width, height, format,
                        type, pixels);
  }

} // namespace gl
