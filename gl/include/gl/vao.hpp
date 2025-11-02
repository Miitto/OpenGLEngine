#pragma once

#include <gl/id.hpp>
#include <glad/glad.h>
#include <optional>

namespace gl {

  /// <summary>
  /// RAII Vertex Array Object wrapper.
  /// </summary>
  class Vao {
    gl::Id m_id = 0;

  public:
    /// <summary>
    /// Creates a new VAO.
    /// </summary>
    inline Vao() { glCreateVertexArrays(1, m_id); }
    /// <summary>
    /// Destroys the VAO.
    /// </summary>
    inline ~Vao() {
      if (m_id != 0)
        glDeleteVertexArrays(1, m_id);
    }
    Vao(const Vao&) = delete;
    Vao& operator=(const Vao&) = delete;
    Vao(Vao&& other) noexcept = default;
    Vao& operator=(Vao&& other) noexcept = default;

    /// <summary>
    /// Returns the VAO handle.
    /// </summary>
    /// <returns>VAO handle</returns>
    inline const gl::Id& id() const { return m_id; }
    /// <summary>
    /// Binds this VAO.
    /// </summary>
    inline void bind() const { glBindVertexArray(m_id); }
    /// <summary>
    /// Unbinds all VAOs.
    /// </summary>
    static void unbind();

    /// <summary>
    /// Bind a vertex buffer to this VAO.
    /// Uses DSA
    ///</summary>
    /// <param name="index"></param> Index to bind the buffer to
    /// <param name="bufferId"></param> Id of the buffer to bind
    /// <param name="offset"></param> Offset in the buffer to start from
    /// <param name="stride"></param> Stride between vertices
    void bindVertexBuffer(GLuint index, const gl::Id& bufferId, GLuint offset,
                          GLuint stride) const;
    /// <summary>
    /// Bind an index buffer to this VAO.
    /// </summary>
    /// <param name="bufferId">The Buffer ID</param>
    void bindIndexBuffer(const gl::Id& bufferId) const;

    /// <summary>
    /// Set the format of a vertex attribute. Will also enable the attribute.
    /// Handles the difference between floats and integers internally.
    /// </summary>
    /// <param name="index">Index of the attribute to set</param>
    /// <param name="numComponents">Number of components of this
    /// attribute</param> <param name="type">GL primitive type of this
    /// attribute</param> <param name="normalize">Whether to normalize the value
    /// to between 0-1 if this attribute is a float</param> <param
    /// name="offset">Offset of this attribute in the vertex (not the
    /// buffer)</param> <param name="bufferIndex">Optional index of the buffer
    /// to bind this attribute to</param>
    void attribFormat(GLuint index, GLuint numComponents, GLenum type,
                      bool normalize, GLuint offset,
                      std::optional<GLuint> bufferIndex = std::nullopt) const;

    /// <summary>
    /// Binds vertex attribute(s) to a given buffer index.
    /// </summary>
    /// <param name="bufferIndex"></param>
    /// <param name="attribIndices"></param>
    void bindAttribs(GLuint bufferIndex,
                     std::initializer_list<GLuint> attribIndices) const;

    void attribDivisor(GLuint index, GLuint divisor) const;

    void label(const char name[]) const;

    /// <summary>
    ///  RAII guard for unbinding a VAO.
    /// </summary>
    class BindGuard {
    public:
      ~BindGuard() { Vao::unbind(); }
    };

    /// <summary>
    /// Binds the VAO and returns a guard that will unbind it on destruction.
    /// </summary>
    /// <returns>RAII object that will unbind the VAO on its
    /// destruction</returns>
    inline BindGuard bindGuard() {
      bind();
      return BindGuard();
    }
  };
  ;
} // namespace gl