#pragma once

#include <gl/bitflag.hpp>
#include <gl/id.hpp>
#include <glad/glad.h>
#include <limits>

namespace gl {
  class Buffer;

  class Mapping {
    gl::Buffer* buffer;
    void* ptr;
    GLuint size;

  public:
    Mapping() = default;
    Mapping(gl::Buffer* buffer, void* ptr, GLuint size = 0,
            bool persistent = false);
    ~Mapping();

    Mapping(const Mapping&) = delete;
    Mapping& operator=(const Mapping&) = delete;
    inline Mapping(Mapping&& o) noexcept
        : buffer(o.buffer), ptr(o.ptr), size(o.size) {
      o.buffer = nullptr;
      o.ptr = nullptr;
      o.size = 0;
    }
    inline Mapping& operator=(Mapping&& o) noexcept {
      if (this != &o) {
        buffer = o.buffer;
        ptr = o.ptr;
        size = o.size;
        o.buffer = nullptr;
        o.ptr = nullptr;
        o.size = 0;
      }
      return *this;
    }

    bool isValid() const;

    operator bool() const { return ptr != nullptr; }
    explicit operator void*() const;
    void* get() const;

    bool isPersistent() const;
    void write(const void* data, GLuint length, GLuint offset = 0) const;
  };

  class MappingRef {
    const Mapping& mapping;
    const GLuint offset = 0;

  public:
    MappingRef(const Mapping& mapping) : mapping(mapping) {}
    MappingRef(const Mapping& mapping, GLuint offset)
        : mapping(mapping), offset(offset) {}

    inline bool isValid() const { return mapping.isValid(); }
    inline operator bool() const { return isValid(); }
    inline void* get() const {
      auto ptr = reinterpret_cast<char*>(mapping.get());
      return ptr + offset;
    }
    inline bool isPersistent() const { return mapping.isPersistent(); }
    inline void write(const void* data, GLuint length,
                      GLuint writeOffset = 0) const {
      mapping.write(data, length, offset + writeOffset);
    }
  };

  /// <summary>
  /// Generic buffer object.
  /// Stores the size of the buffer.
  /// </summary>
  class Buffer {
  protected:
    gl::Id m_id = 0;
    GLuint m_size = 0;
    // Topmost bit is set when persistently mapped
    void* m_mapping = nullptr;

  public:
    constexpr inline static GLuint roundToAlignment(GLuint size,
                                                    GLuint alignment) {
#ifndef NDEBUG
      if (alignment == 0)
        throw std::invalid_argument("Alignment cannot be zero");
#endif

      GLuint remainder = size % alignment;
      if (remainder == 0)
        return size;
      return size + (alignment - remainder);
    }

    /// <summary>
    /// Buffer usage flags.
    /// </summary>
    enum class Usage {
      DEFAULT = 0,
      /// <summary>
      /// Allows the buffer to be read from CPU.
      /// </summary>
      READ = GL_MAP_READ_BIT,
      /// <summary>
      /// Allows the buffer to be written to by CPU.
      /// </summary>
      WRITE = GL_MAP_WRITE_BIT,
      /// <summary>
      /// Hint to use dynamic storage, allowing updates without needing a
      /// copy-buffer.
      /// </summary>
      DYNAMIC = GL_DYNAMIC_STORAGE_BIT,
      /// <summary>
      /// Allows the buffer to be used while being mapped.
      /// </summary>
      PERSISTENT = GL_MAP_PERSISTENT_BIT,
      /// <summary>
      /// Makes the driver keep the buffer coherent between CPU and GPU.
      /// </summary>
      COHERENT = GL_MAP_COHERENT_BIT,
      /// <summary>
      /// Allow for explicit flushing of modified ranges.
      /// </summary>
      FLUSH_EXPLICIT = GL_MAP_FLUSH_EXPLICIT_BIT
    };
    using UsageBitFlag = Bitflag<Usage>;

    /// <summary>
    /// Creates a new buffer handle. Buffer will have no storage.
    /// </summary>
    inline Buffer() { glCreateBuffers(1, m_id); }

    /// <summary>
    /// Creates a new buffer with the given storage size and flags.
    /// </summary>
    /// <param name="size">Size for the buffer.</param>
    /// <param name="data">Pointer to copy into the buffer from. Will not copy
    /// from nullptr.</param>
    /// <param name="usage">Buffer usage flags.</param>
    inline Buffer(GLuint size, const void* data = nullptr,
                  UsageBitFlag usage = Usage::DEFAULT)
        : Buffer() {
      m_size = size;
      glNamedBufferStorage(m_id, size, data, usage);
    }

    /// <summary>
    /// Buffer mapping flags.
    /// Should not contain flags that were not used during buffer creation.
    /// </summary>
    enum class Mapping {
      /// <summary>
      /// Allow reading from the mapping.
      /// </summary>
      READ = GL_MAP_READ_BIT,
      /// <summary>
      /// Allow writing to the mapping.
      /// </summary>
      WRITE = GL_MAP_WRITE_BIT,
      /// <summary>
      /// Allows the buffer to be used while being mapped.
      /// </summary>
      PERSISTENT = GL_MAP_PERSISTENT_BIT,
      /// <summary>
      /// Keeps the buffer coherent between CPU and GPU.
      /// </summary>
      COHERENT = GL_MAP_COHERENT_BIT,
      /// <summary>
      /// Invalidates the buffer
      /// </summary>
      INVALIDATE_BUFFER = GL_MAP_INVALIDATE_BUFFER_BIT,
      /// <summary>
      /// Invalidates part of the buffer being mapped.
      /// </summary>
      INVALIDATE_RANGE = GL_MAP_INVALIDATE_RANGE_BIT,
      /// <summary>
      /// Allows the buffer to be flushed explicitly.
      /// </summary>
      FLUSH_EXPLICIT = GL_MAP_FLUSH_EXPLICIT_BIT,
      /// <summary>
      /// Disables implicit synchronization when mapping.
      /// </summary>
      UNSYNCHRONIZED = GL_MAP_UNSYNCHRONIZED_BIT
    };
    using MappingBitFlag = Bitflag<Mapping>;

    /// <summary>
    /// Check if the buffer is valid (has been created).
    /// </summary>
    /// <returns>True if the buffer references a valid buffer.</returns>
    inline bool isValid() const { return m_id != 0; }

    /// <summary>
    /// Buffer destructor.
    /// Frees the buffer if the handle is valid.
    /// </summary>
    inline ~Buffer() {
      if (m_id != 0)
        glDeleteBuffers(1, m_id);
    }

    /// <summary>
    /// Initialize the buffer with given size, data and usage flags.
    /// MUST only be called ONCE per buffer.
    /// </summary>
    /// <param name="size">Size of the buffer</param>
    /// <param name="data">Pointer to data to write if not nullptr.</param>
    /// <param name="usage">Buffer usage flags.</param>
    void init(GLuint size, const void* data = nullptr,
              UsageBitFlag usage = Usage::DEFAULT);

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&& other) noexcept = default;
    Buffer& operator=(Buffer&& other) noexcept = default;

    /// <summary>
    /// Get the buffer handle.
    /// </summary>
    /// <returns>The buffer handle</returns>
    inline const gl::Id& id() const { return m_id; }
    /// <summary>
    /// Unbind all buffers from the given target.
    /// </summary>
    /// <param name="target"></param>
    static void unbind(GLenum target);

    /// <summary>
    /// Create a mapping of the buffer with given flags, offset and length.
    /// </summary>
    /// <param name="flags">Flags to use when mapping the buffer.</param>
    /// <param name="offset">Offset into the buffer to map. Defaults to
    /// 0.</param> <param name="length">Size of the buffer to map. Defaults to
    /// max.</param> <returns></returns>
    gl::Mapping map(MappingBitFlag flags, GLuint offset = 0,
                    GLuint length = std::numeric_limits<GLuint>::max());
    /// <summary>
    /// Unmaps the buffer.
    /// </summary>
    void unmap();

    /// <summary>
    /// Retrieves the current persistent mapping of the buffer, if any. May not
    /// return a valid mapping.
    /// </summary>
    /// <returns></returns>
    const gl::Mapping getMapping() const;

    /// <summary>
    /// Set the debug label of this buffer.
    /// </summary>
    /// <param name="name">Name to give the buffer.</param>
    void label(const char name[]) const;

    /// <summary>
    /// Write data from one buffer to another.
    /// Allows for writes to an immutable buffer.
    /// </summary>
    /// <param name="target">Buffer to write to</param>
    /// <param name="readOffset">Offset into this buffer to read from</param>
    /// <param name="writeOffset">Offset into the target buffer to write
    /// to</param> <param name="size">Size to copy over</param>
    void copyTo(const gl::Buffer& target, GLuint readOffset, GLuint writeOffset,
                GLuint size) const;

    /// <summary>
    /// Write to this buffer. Must have been created with
    /// gl::Buffer::Usage::DYNAMIC
    /// </summary>
    /// <param name="offset">Offset to write at</param>
    /// <param name="size">Size to write</param>
    /// <param name="data">Pointer to the data to write</param>
    void subData(GLuint offset, GLuint size, const void* data) const;

    enum class BasicTarget {
      ARRAY = GL_ARRAY_BUFFER,
      ATOMIC_COUNTER = GL_ATOMIC_COUNTER_BUFFER,
      COPY_READ = GL_COPY_READ_BUFFER,
      COPY_WRITE = GL_COPY_WRITE_BUFFER,
      DISPATCH_INDIRECT = GL_DISPATCH_INDIRECT_BUFFER,
      DRAW_INDIRECT = GL_DRAW_INDIRECT_BUFFER,
      ELEMENT_ARRAY = GL_ELEMENT_ARRAY_BUFFER,
      PIXEL_PACK = GL_PIXEL_PACK_BUFFER,
      PIXEL_UNPACK = GL_PIXEL_UNPACK_BUFFER,
      QUERY = GL_QUERY_BUFFER,
      SHADER_STORAGE = GL_SHADER_STORAGE_BUFFER,
      TEXTURE = GL_TEXTURE_BUFFER,
      TRANSFORM_FEEDBACK = GL_TRANSFORM_FEEDBACK_BUFFER,
    };

    using BasicTargetBitFlag = Bitflag<BasicTarget>;

    /// <summary>
    /// Bind the buffer to the given target.
    /// </summary>
    /// <param name="target">Target to bind to.</param>
    inline void bind(BasicTargetBitFlag target) const {
      glBindBuffer(target, m_id);
    }

    enum class StorageTarget {
      UNIFORM = GL_UNIFORM_BUFFER,
      STORAGE = GL_SHADER_STORAGE_BUFFER
    };

    using StorageTargetBitFlag = Bitflag<StorageTarget>;

    /// <summary>
    /// Binds the entire buffer to the given target at the given index.
    /// </summary>
    /// <param name="target">Target to bind the buffer to.</param>
    /// <param name="index">Binding to bind the buffer at.</param>
    inline void bindBase(StorageTargetBitFlag target, GLuint index) const {
      glBindBufferBase(target, index, m_id);
    }
    /// <summary>
    /// Bind part of the buffer to the given target at the given index.
    /// </summary>
    /// <param name="target">Target to bind the buffer to.</param>
    /// <param name="index">Binding to bind the buffer at.</param>
    /// <param name="offset">Offset into the buffer. Must be a multiple of
    /// GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT.</param>
    /// <param name="size">Size after
    /// the offset. Must be at least GL_UNIFORM_BLOCK_SIZE_DATA.</param>
    inline void bindRange(StorageTargetBitFlag target, GLuint index,
                          GLuint offset, GLuint size) const {
      glBindBufferRange(target, index, m_id, offset, size);
    }
  };
} // namespace gl