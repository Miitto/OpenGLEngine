#pragma once

#include <gl/id.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace gl {
  /// <summary>
  /// RAII 2D Texture wrapper
  /// </summary>
  class Texture {
  public:
    /// <summary>
    /// 2D Texture size
    /// </summary>
    struct Size {
      GLsizei width;
      GLsizei height;
    };

  private:
    gl::Id m_id = 0;
    gl::Texture::Size m_size{};

  public:
    /// <summary>
    /// Creates a new texture without storage.
    /// </summary>
    Texture() { glCreateTextures(GL_TEXTURE_2D, 1, m_id); }
    /// <summary>
    /// Creates a new texture with the given size, format, internal format
    /// </summary>
    /// <param name="size">Texture dimensions</param>
    /// <param name="format">Texture format (RGBA)</param>
    /// <param name="internalFormat">Internal Format (RGBA8)</param>
    /// <param name="data">Pointer to texture data</param>
    Texture(gl::Texture::Size size, GLenum format, GLenum internalFormat,
            void* data) {
      glCreateTextures(GL_TEXTURE_2D, 1, m_id);

      storage(1, internalFormat, size);
      subImage(0, 0, 0, size.width, size.height, format, GL_UNSIGNED_BYTE,
               data);
    }
    /// <summary>
    /// Returns the texture ID.
    /// </summary>
    /// <returns>Texture handle</returns>
    const gl::Id& id() const { return m_id; }

    /// <summary>
    /// Labels the texture for debugging purposes.
    /// </summary>
    /// <param name="name">Texture name</param>
    inline void label(const char* name) const {
      glObjectLabel(GL_TEXTURE, m_id, -1, name);
    }

    /// <summary>
    /// Creates mipmaps for the texture.
    /// </summary>
    inline void generateMipmap() const { glGenerateTextureMipmap(m_id); }
    /// <summary>
    /// Returns the texture handle for use with bindless textures.
    /// </summary>
    /// <returns></returns>
    inline GLuint64 getHandle() const { return glGetTextureHandleARB(m_id); }
    /// <summary>
    /// Makes the texture handle resident for bindless textures.
    /// </summary>
    inline void makeResident() const { makeHandleResident(getHandle()); }
    /// <summary>
    /// Makes the texture handle non-resident for bindless textures.
    /// </summary>
    inline void makeNonResident() const { makeHandleNonResident(getHandle()); }

    /// <summary>
    /// Makes the given texture handle resident.
    /// </summary>
    /// <param name="handle">Texture handle</param>
    inline static void makeHandleResident(GLuint64 handle) {
      glMakeTextureHandleResidentARB(handle);
    }
    /// <summary>
    /// Makes the given texture handle non-resident.
    /// </summary>
    /// <param name="handle">Texture handle</param>
    inline static void makeHandleNonResident(GLuint64 handle) {
      glMakeTextureHandleNonResidentARB(handle);
    }

    /// <summary>
    /// Converts a number of channels to an OpenGL format.
    /// </summary>
    /// <param name="channels">Number of channels</param>
    /// <returns>Texture format</returns>
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

    /// <summary>
    /// Converts a number of channels to an OpenGL internal format.
    /// </summary>
    /// <param name="channels">Number of channels</param>
    /// <returns>Internal texture format</returns>
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

    /// <summary>
    /// Binds the texture to the given texture unit.
    /// </summary>
    /// <param name="unit">Unit to bind to</param>
    void bind(uint8_t unit) const { glBindTextureUnit(unit, m_id); }
    /// <summary>
    /// Unbinds all textures from the given texture unit.
    /// </summary>
    /// <param name="unit"></param>
    static void unbind(GLenum unit) { glBindTextureUnit(unit, 0); }
    /// <summary>
    /// Sets a texture parameter.
    /// </summary>
    /// <param name="pname">Parameter name</param>
    /// <param name="param">Parameter value</param>
    void setParameter(GLenum pname, GLint param) const {
      glTextureParameteri(m_id, pname, param);
    }
    /// <summary>
    /// Creates storage for the texture.
    /// MUST only be called ONCE per texture level.
    /// </summary>
    /// <param name="level">Texture level</param>
    /// <param name="internalformat">Internal texture format</param>
    /// <param name="size">Texture dimensions</param>
    void storage(GLint level, GLenum internalformat, gl::Texture::Size size) {
      m_size = size;
      glTextureStorage2D(m_id, level, internalformat, size.width, size.height);
    }
    /// <summary>
    /// Writes pixels to a sub-region of the texture.
    /// </summary>
    /// <param name="level">Texture level</param>
    /// <param name="xoffset">X Offset</param>
    /// <param name="yoffset">Y Offset</param>
    /// <param name="width">Width</param>
    /// <param name="height">Height</param>
    /// <param name="format">Texture format</param>
    /// <param name="type">Data type</param>
    /// <param name="pixels">Pointer to pixel data</param>
    void subImage(GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                  GLsizei height, GLenum format, GLenum type,
                  const void* pixels) const {
      glTextureSubImage2D(m_id, level, xoffset, yoffset, width, height, format,
                          type, pixels);
    }

    /// <summary>
    /// Get the texture dimensions.
    /// </summary>
    /// <returns>Texture dimensions</returns>
    const gl::Texture::Size& size() const { return m_size; }
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

  class TextureArray {
  public:
    struct Size {
      GLsizei width;
      GLsizei height;
      GLsizei depth;
    };

  protected:
    gl::Id m_id = 0;
    Size m_size{};

  public:
    TextureArray() { glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, m_id); }
    TextureArray(Size size, GLenum format, GLenum internalFormat, void* data) {
      glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, m_id);

      storage(1, internalFormat, size);
      subImage(0, 0, 0, 0, size.width, size.height, size.depth, format,
               GL_UNSIGNED_BYTE, data);
    }
    const gl::Id& id() const { return m_id; }

    inline void label(const char* name) const {
      glObjectLabel(GL_TEXTURE, m_id, -1, name);
    }

    inline void generateMipmap() const { glGenerateTextureMipmap(m_id); }

    void bind(uint8_t unit) const { glBindTextureUnit(unit, m_id); }
    static void unbind(GLenum unit) { glBindTextureUnit(unit, 0); }
    void setParameter(GLenum pname, GLint param) const {
      glTextureParameteri(m_id, pname, param);
    }
    void storage(GLint level, GLenum internalformat, Size size) {
      m_size = size;
      glTextureStorage3D(m_id, level, internalformat, size.width, size.height,
                         size.depth);
    }
    void subImage(GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
                  GLsizei width, GLsizei height, GLsizei depth, GLenum format,
                  GLenum type, const void* pixels) const {
      glTextureSubImage3D(m_id, level, xoffset, yoffset, zoffset, width, height,
                          depth, format, type, pixels);
    }

    const Size& size() const { return m_size; }
  };
} // namespace gl