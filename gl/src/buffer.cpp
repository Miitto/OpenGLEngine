#include "logger.hpp"
#include <gl/buffer.hpp>

namespace {
  /// <summary>
  /// Bit mask for the persistent mapping bit in a pointer.
  /// Since only the lower 48 bits are used for addressing, the topmost bits are
  /// safe to use as flags (so long as they are cleared before being
  /// dereferenced).
  /// </summary>
  constexpr intptr_t PERSISTENT_MAP_BIT = 1LL << 63L;

  /// <summary>
  /// Returns the given pointer with the persistent (topmost) bit set.
  /// The returned pointer MUST NOT be dereferenced directly.
  /// </summary>
  /// <param name="ptr">In ptr</param>
  /// <returns>In ptr with the persistent (topmost) bit set</returns>
  inline void* setPersistentBit(void* ptr) {
    intptr_t iptr = reinterpret_cast<intptr_t>(ptr);
    iptr |= PERSISTENT_MAP_BIT;
    return reinterpret_cast<void*>(iptr);
  }

  /// <summary>
  /// Returns the given pointer with the persistent (topmost) bit cleared.
  /// The returned pointer is safe (in regards to the persistent bit) to
  /// dereference.
  /// </summary>
  /// <param name="ptr">In ptr</param>
  /// <returns>In ptr with the persistent (topmost) bit cleared.</returns>
  inline void* clearPersistentBit(void* ptr) {
    intptr_t iptr = reinterpret_cast<intptr_t>(ptr);
    iptr &= ~PERSISTENT_MAP_BIT;
    return reinterpret_cast<void*>(iptr);
  }
} // namespace

namespace gl {

  Mapping::Mapping(gl::Buffer* buffer, void* ptr, GLuint size, GLuint offset,
                   bool persistent)
      : buffer(buffer), ptr(ptr), size(size), offset(offset) {
    if (persistent) {
      this->ptr = setPersistentBit(ptr);
    }
  }

  Mapping::~Mapping() {
    if (isValid() && !isPersistent()) {
      buffer->unmap();
    }
  }

  bool Mapping::isValid() const {
    return clearPersistentBit(ptr) != nullptr && buffer != nullptr;
  }

  bool Mapping::isPersistent() const {
    return (reinterpret_cast<intptr_t>(ptr) & PERSISTENT_MAP_BIT) != 0;
  }

  void* Mapping::get() const { return clearPersistentBit(ptr); }
  Mapping::operator void*() const { return get(); }

  void Mapping::write(const void* data, GLuint length, GLuint offset) const {
    auto ptr = reinterpret_cast<char*>(get());
#ifndef NDEBUG
    if (data == nullptr) {
      gl::Logger::error("Attempted to write null data to mapped buffer");
      return;
    }

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

  void Mapping::flush(GLuint length, GLuint offset) const {
    glFlushMappedNamedBufferRange(buffer->id(), offset + this->offset, length);
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

  gl::Mapping gl::Buffer::map(MappingBitFlag flags, GLuint offset,
                              GLuint length) {
#ifndef NDEBUG
    if (m_id == 0) {
      gl::Logger::error("Attempted to map an uninitialized buffer");
    }
#endif

    length = length == std::numeric_limits<GLuint>::max() ? m_size : length;

    auto ptr = glMapNamedBufferRange(m_id, offset, length, flags);

    return gl::Mapping(this, ptr, length, offset,
                       (flags & GL_MAP_PERSISTENT_BIT) != 0);
  }

  inline void gl::Buffer::unmap() {
#ifndef NDEBUG
    if (m_id == 0) {
      gl::Logger::error("Attempted to unmap an uninitialized buffer");
    }
#endif
    glUnmapNamedBuffer(m_id);
  }

  void gl::Buffer::unbind(GLenum target) { glBindBuffer(target, 0); }

  void gl::Buffer::label(const char name[]) const {
    glObjectLabel(GL_BUFFER, m_id, -1, name);
  }

  void gl::Buffer::copyTo(const gl::Buffer& target, GLuint readOffset,
                          GLuint writeOffset, GLuint size) const {
    glCopyNamedBufferSubData(m_id, target.m_id, readOffset, writeOffset, size);
  }

  void gl::Buffer::subData(GLuint offset, GLuint size, const void* data) const {
    glNamedBufferSubData(m_id, offset, size, data);
  }
} // namespace gl
