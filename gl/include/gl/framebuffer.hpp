#pragma once

#include <gl/id.hpp>
#include <gl/texture.hpp>
#include <glad/glad.h>

namespace gl {
  /// <summary>
  /// RAII Framebuffer object wrapper.
  /// </summary>
  class Framebuffer {
    gl::Id m_id = gl::Id(0);

    explicit constexpr Framebuffer(size_t s) : m_id(0) { (void)s; }

  public:
    /// <summary>
    /// Creates a new framebuffer object.
    /// </summary>
    Framebuffer() { glCreateFramebuffers(1, m_id); }
    ~Framebuffer() {
      if (m_id != 0)
        glDeleteFramebuffers(1, m_id);
    }

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&& other) noexcept {
      if (m_id != 0)
        glDeleteFramebuffers(1, m_id);
      m_id = std::move(other.m_id);
      other.m_id = gl::Id(0);
    }

    Framebuffer& operator=(Framebuffer&& other) noexcept {
      if (this != &other) {
        if (m_id != 0)
          glDeleteFramebuffers(1, m_id);
        m_id = std::move(other.m_id);
        other.m_id = gl::Id(0);
      }
      return *this;
    }

    static constexpr Framebuffer uninitialized() { return Framebuffer(0); }

    /// <summary>
    /// Gets the framebuffer ID.
    /// </summary>
    /// <returns>Framebuffer ID</returns>
    const gl::Id& id() const { return m_id; }

    /// <summary>
    /// Attaches a texture to the framebuffer at the given attachment point.
    /// </summary>
    /// <param name="attachment">Attachment point</param>
    /// <param name="texture">Texture to attach</param>
    /// <param name="level">Level of attachment</param>
    void attachTexture(GLenum attachment, const gl::Texture& texture,
                       GLint level = 0) const {
      glNamedFramebufferTexture(m_id, attachment, texture.id(), level);
    }

    void attachTexture(GLenum attachment, GLuint textureId,
                       GLint level = 0) const {
      glNamedFramebufferTexture(m_id, attachment, textureId, level);
    }

    void attachTextureLayer(GLenum attachment, const gl::Texture& texture,
                            GLint level, GLint layer) const {
      glNamedFramebufferTextureLayer(m_id, attachment, texture.id(), level,
                                     layer);
    }

    void attachTextureLayer(GLenum attachment, GLuint textureId, GLint level,
                            GLint layer) const {
      glNamedFramebufferTextureLayer(m_id, attachment, textureId, level, layer);
    }

    /// <summary>
    /// Binds the framebuffer to the given target.
    /// </summary>
    /// <param name="target">Target to bind to. Defaults to GL_FRAMEBUFFER (Read
    /// and Draw)</param>
    void bind(GLenum target = GL_FRAMEBUFFER) const {
      glBindFramebuffer(target, m_id);
    }
    /// <summary>
    /// Binds the framebuffer as the read framebuffer.
    /// </summary>
    void bindRead() const { glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id); }
    /// <summary>
    /// Binds the framebuffer as the draw framebuffer.
    /// </summary>
    void bindDraw() const { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_id); }
    /// <summary>
    /// Unbinds all framebuffers from the given target.
    /// Returns to the default framebuffer.
    /// </summary>
    /// <param name="target"></param>
    static void unbind(GLenum target = GL_FRAMEBUFFER) {
      glBindFramebuffer(target, 0);
    }

    /// <summary>
    /// Blits from this framebuffer to the target framebuffer.
    /// </summary>
    /// <param name="target">Target framebuffer to blit to</param>
    /// <param name="srcX0">src x min</param>
    /// <param name="srcY0">src y min</param>
    /// <param name="srcX1">src x max</param>
    /// <param name="srcY1">src y max</param>
    /// <param name="dstX0">dst x min</param>
    /// <param name="dstY0">dst y min</param>
    /// <param name="dstX1">dst x max</param>
    /// <param name="dstY1">dst y max</param>
    /// <param name="mask">Channel mask</param>
    /// <param name="filter">Texture filter</param>
    void blit(GLuint target, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
              GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
              GLbitfield mask = 0, GLenum filter = 0) const {
      glBlitNamedFramebuffer(m_id, target, srcX0, srcY0, srcX1, srcY1, dstX0,
                             dstY0, dstX1, dstY1, mask, filter);
    }
  };
} // namespace gl