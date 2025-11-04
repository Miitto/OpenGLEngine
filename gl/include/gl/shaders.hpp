#pragma once

#include <expected>
#include <gl/id.hpp>
#include <glad/glad.h>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace gl {
  /// <summary>
  /// RAII Shader object wrapper.
  /// Created from source or from file.
  /// </summary>
  class Shader {
    gl::Id m_id;

  public:
    enum Type {
      VERTEX = GL_VERTEX_SHADER,
      FRAGMENT = GL_FRAGMENT_SHADER,
      GEOMETRY = GL_GEOMETRY_SHADER,
      TESS_CONTROL = GL_TESS_CONTROL_SHADER,
      TESS_EVAL = GL_TESS_EVALUATION_SHADER,
      COMPUTE = GL_COMPUTE_SHADER
    };

    Shader(Type type, std::string_view source);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&&) noexcept = default;
    Shader& operator=(Shader&&) noexcept = default;

    const gl::Id& id() const { return m_id; }

    /// <summary>
    /// Creates a shader from a file at the given path.
    /// </summary>
    /// <param name="path">Path to read file from</param>
    /// <param name="type">Shader type</param>
    /// <returns>Shader on successful read</returns>
    static std::optional<Shader> fromFile(std::string_view path, Type type);
  };

  /// <summary>
  /// RAII wrapper for a linked program.
  /// </summary>
  class Program {
    gl::Id m_id = 0;

    /// <summary>
    /// Handles reading the link error log if linking failed.
    /// </summary>
    /// <param name="id">Program ID</param>
    /// <returns>If linking failed.</returns>
    static bool handleLinkFail(const gl::Id&& id);

    Program(gl::Id&& id) : m_id(std::move(id)) {}

  public:
    explicit Program() = default;
    inline bool isValid() const { return m_id != 0; }
    inline GLuint id() const { return m_id; }

    /// <summary>
    /// Creates a program from the given shaders.
    /// </summary>
    /// <param name="...shaders">List of shaders</param>
    /// <returns>Program on link success</returns>
    template <typename... Shaders>
      requires(std::is_same<Shaders, Shader>::value && ...)
    inline static std::optional<Program> create(Shaders&... shaders) {
      GLuint id = glCreateProgram();
      (glAttachShader(id, shaders.id()), ...);
      glLinkProgram(id);
      if (handleLinkFail(id)) {

        return std::nullopt;
      }
      return Program(gl::Id(id));
    }

    /// <summary>
    /// Creates a program from the given span of shaders.
    /// </summary>
    /// <param name="shaders">List of shaders</param>
    /// <returns>Program on link success</returns>
    inline static std::optional<Program> create(std::span<Shader>&& shaders) {
      GLuint id = glCreateProgram();
      for (const auto& shader : shaders) {
        glAttachShader(id, shader.id());
      }
      glLinkProgram(id);
      if (handleLinkFail(id)) {
        return std::nullopt;
      }
      return Program(gl::Id(id));
    }

    /// <summary>
    /// Binds the current program.
    /// </summary>
    void bind() const { glUseProgram(m_id); }

    /// <summary>
    /// Creates shaders from the given file paths and types, then links them
    /// into a program.
    /// </summary>
    /// <param name="paths">List of {file_path, shader_type} for each
    /// shader.</param> <returns>Program on success, string of the error on
    /// fail</returns>
    static std::expected<Program, std::string>
    fromFiles(std::initializer_list<std::pair<std::string_view, Shader::Type>>
                  paths) {
      std::vector<Shader> shaders;
      shaders.reserve(paths.size());
      for (const auto& [path, type] : paths) {
        auto shaderOpt = Shader::fromFile(path, type);
        if (!shaderOpt.has_value()) {
          return std::unexpected(std::string("Failed to load shader from ") +
                                 std::string(path));
        }
        shaders.push_back(std::move(shaderOpt.value()));
      }

      auto programOpt = create(shaders);
      if (!programOpt.has_value()) {
        return std::unexpected("Failed to link program");
      }
      return std::move(programOpt.value());
    }
  };
} // namespace gl