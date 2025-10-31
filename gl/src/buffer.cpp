#include "logger.hpp"
#include <gl/buffer.hpp>

namespace {
  constexpr intptr_t PERSISTENT_MAP_BIT = 1LL << 63L;

  inline void* setPersistentBit(void* ptr) {
    intptr_t iptr = reinterpret_cast<intptr_t>(ptr);
    iptr |= PERSISTENT_MAP_BIT;
    return reinterpret_cast<void*>(iptr);
  }

  inline void* clearPersistentBit(void* ptr) {
    intptr_t iptr = reinterpret_cast<intptr_t>(ptr);
    iptr &= ~PERSISTENT_MAP_BIT;
    return reinterpret_cast<void*>(iptr);
  }
} // namespace

namespace gl {

  Mapping::Mapping(gl::Buffer* buffer, void* ptr, GLuint size, bool persistent)
      : buffer(buffer), ptr(ptr), size(size) {
    if (persistent) {
      this->ptr = setPersistentBit(ptr);
    }
  }

  Mapping::~Mapping() {
    if (isValid() && !isPersistent()) {
      buffer->unmap();
    }
  }

  bool Mapping::isPersistent() const {
    return (reinterpret_cast<intptr_t>(ptr) & PERSISTENT_MAP_BIT) != 0;
  }

  void* Mapping::get() const { return clearPersistentBit(ptr); }
  Mapping::operator void*() const { return get(); }

  void Mapping::write(const void* data, GLuint length, GLuint offset) const {
#ifndef NDEBUG
    if (data == nullptr) {
      gl::Logger::error("Attempted to write null data to mapped buffer");
      return;
    }
    auto ptr = reinterpret_cast<char*>(get());

    if (ptr == nullptr) {
      gl::Logger::error("Attempted to write to unmapped buffer");
      return;
    }

    if (offset + length > size) {
      gl::Logger::error("Attempted to write beyond mapped range");
      return;
    }
#endif
    char* dst = ptr + offset;
    memcpy(dst, data, length);
  }

  void gl::Buffer::init(GLuint size, const void* data, UsageBitFlag flags) {
#ifndef NDEBUG
    if (m_size != 0) {
      gl::Logger::error("Attempted to reinitialize a buffer");
      return;
    }
#endif
    m_size = size;
    glNamedBufferStorage(m_id, size, data, flags);
  }

  const gl::Mapping gl::Buffer::map(MappingBitFlag flags, GLuint offset,
                                    GLuint length) {
    length = length == std::numeric_limits<GLuint>::max() ? m_size : length;

    auto ptr = glMapNamedBufferRange(m_id, offset, length, flags);
    if ((flags & GL_MAP_PERSISTENT_BIT) != 0) {
      m_mapping = setPersistentBit(ptr);
    }

    return gl::Mapping(this, ptr, length, (flags & GL_MAP_PERSISTENT_BIT) != 0);
  }

  inline void gl::Buffer::unmap() {
    glUnmapNamedBuffer(m_id);
    m_mapping = {};
  }

  /// <summary>
  /// Retrive the current persistent mapping of the buffer, if any.
  /// Mapping will be invalid if the buffer is not persistently mapped.
  /// </summary>
  /// <returns></returns>
  const gl::Mapping gl::Buffer::getMapping() const {
    // Yucky const_cast, but preferable to needing a mutable buffer to retrieve
    // the mapping
    return gl::Mapping(
        const_cast<gl::Buffer*>(this), const_cast<void*>(m_mapping), m_size,
        (reinterpret_cast<uintptr_t>(m_mapping) & PERSISTENT_MAP_BIT) != 0);
  }

  void gl::Buffer::unbind(GLenum target) { glBindBuffer(target, 0); }

  void gl::Buffer::label(const char name[]) const {
    glObjectLabel(GL_BUFFER, m_id, -1, name);
  }
} // namespace gl
