#pragma once

#include <gl/id.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace gl {
  using RawTextureHandle = GLuint64;

  class TextureHandle {
  public:
    inline TextureHandle(RawTextureHandle handle) : _handle(handle) {}

    void use();
    void unuse();

    inline const RawTextureHandle& handle() const { return _handle; }
    inline operator RawTextureHandle() const { return _handle; }

    inline bool isValid() const { return _handle != 0; }

  protected:
    RawTextureHandle _handle;
  };

  class Sampler {
  public:
    inline Sampler() { glCreateSamplers(1, m_id); }
    ~Sampler() {
      if (m_id != 0)
        glDeleteSamplers(1, m_id);
    }
    Sampler(const Sampler&) = delete;
    Sampler& operator=(const Sampler&) = delete;
    Sampler(Sampler&& other) noexcept = default;
    Sampler& operator=(Sampler&& other) noexcept = default;

    inline const gl::Id& id() const { return m_id; }
    inline void setParameter(GLenum pname, GLint param) const {
      glSamplerParameteri(m_id, pname, param);
    }

    inline void bind(GLuint unit) const { glBindSampler(unit, m_id); }
    inline static void unbind(GLuint unit) { glBindSampler(unit, 0); }

  protected:
    gl::Id m_id = 0;
  };

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
    gl::TextureHandle _handle = 0;

  public:
    /// <summary>
    /// Creates a new texture without storage.
    /// </summary>
    inline Texture() { glCreateTextures(GL_TEXTURE_2D, 1, m_id); }
    /// <summary>
    /// Creates a new texture with the given size, format, internal format
    /// </summary>
    /// <param name="size">Texture dimensions</param>
    /// <param name="format">Texture format (RGBA)</param>
    /// <param name="internalFormat">Internal Format (RGBA8)</param>
    /// <param name="data">Pointer to texture data</param>
    Texture(gl::Texture::Size size, GLenum format, GLenum internalFormat,
            void* data);

    ~Texture() {
      if (m_id != 0)
        glDeleteTextures(1, m_id);
    }

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept = default;
    Texture& operator=(Texture&& other) noexcept = default;

    inline bool isValid() const { return m_id != 0; }

    /// <summary>
    /// Returns the texture ID.
    /// </summary>
    /// <returns>Texture handle</returns>
    inline const gl::Id& id() const { return m_id; }

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

    inline void createHandle() {
      RawTextureHandle rawHandle = glGetTextureHandleARB(m_id);
      _handle = TextureHandle(rawHandle);
    }

    /// <summary>
    /// Returns the texture handle for use with bindless textures.
    /// Should not be written to a buffer, use rawHandle() for that.
    /// createHandle() must be called first.
    /// </summary>
    /// <returns>TextureHandle of this texture</returns>
    inline TextureHandle handle() const { return _handle; }
    /// <summary>
    /// Returns the raw texture handle for use with bindless textures.
    /// createHandle() must be called first.
    /// </summary>
    /// <returns>Raw Texture handle to be written</returns>
    inline RawTextureHandle rawHandle() const { return _handle.handle(); }

    /// <summary>
    /// Converts a number of channels to an OpenGL format.
    /// </summary>
    /// <param name="channels">Number of channels</param>
    /// <returns>Texture format</returns>
    static GLenum formatFromChannels(int channels);
    /// <summary>
    /// Converts a number of channels to an OpenGL internal format.
    /// </summary>
    /// <param name="channels">Number of channels</param>
    /// <returns>Internal texture format</returns>
    static GLenum internalFormatFromChannels(int channels);
    /// <summary>
    /// Binds the texture to the given texture unit.
    /// </summary>
    /// <param name="unit">Unit to bind to</param>
    inline void bind(uint8_t unit) const { glBindTextureUnit(unit, m_id); }
    /// <summary>
    /// Unbinds all textures from the given texture unit.
    /// </summary>
    /// <param name="unit"></param>
    inline static void unbind(GLenum unit) { glBindTextureUnit(unit, 0); }
    /// <summary>
    /// Sets a texture parameter.
    /// </summary>
    /// <param name="pname">Parameter name</param>
    /// <param name="param">Parameter value</param>
    inline void setParameter(GLenum pname, GLint param) const {
      glTextureParameteri(m_id, pname, param);
    }
    /// <summary>
    /// Creates storage for the texture.
    /// MUST only be called ONCE per texture level.
    /// </summary>
    /// <param name="level">Texture level</param>
    /// <param name="internalformat">Internal texture format</param>
    /// <param name="size">Texture dimensions</param>
    void storage(GLint level, GLenum internalformat, gl::Texture::Size size);
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
                  const void* pixels) const;
    /// <summary>
    /// Get the texture dimensions.
    /// </summary>
    /// <returns>Texture dimensions</returns>
    inline const gl::Texture::Size& size() const { return m_size; }
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