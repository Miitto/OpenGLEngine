#pragma once

#include <expected>
#include <gl/texture.hpp>
#include <glm/glm.hpp>
#include <stb/image.h>
#include <string_view>

namespace engine {
  /// <summary>
  /// An image loaded from disk.
  /// </summary>
  class Image {
    /// <summary>
    /// Whether stb_image has been initialized.
    /// </summary>
    static bool inited;
    /// <summary>
    /// Initializes stb_image on first use, NOOP otherwise.
    /// </summary>
    static void ensureInit();

    Image(glm::ivec2 dim, int channels, unsigned char* data)
        : dimensions(dim), channels(channels), data(data) {}

  public:
    Image() = delete;
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    Image(Image&& other) noexcept
        : dimensions(other.dimensions), channels(other.channels),
          data(other.data) {
      other.data = nullptr;
    }
    Image& operator=(Image&& other) noexcept {
      if (this != &other) {
        stbi_image_free(data);
        dimensions = other.dimensions;
        channels = other.channels;
        data = other.data;
        other.data = nullptr;
      }
      return *this;
    }

    /// <summary>
    /// Creates an image from a file.
    /// </summary>
    /// <param name="file">File path</param>
    /// <param name="desiredChannels">Number of channels to use, 0 to use image
    /// defined.</param>
    /// <returns>Image on success, error string on failure</returns>
    static inline std::expected<Image, std::string>
    fromFile(std::string_view file, int desiredChannels = 0) {
      ensureInit();
      int width, height, channels;
      unsigned char* data =
          stbi_load(file.data(), &width, &height, &channels, desiredChannels);
      if (data == nullptr) {
        auto err = stbi_failure_reason();
        return std::unexpected(std::string("Failed to load image from ") +
                               file.data() + "\n" + err);
      }
      Image img(glm::ivec2(width, height), channels, data);
      return img;
    }

    ~Image() { stbi_image_free(data); }

    inline bool isValid() const { return data != nullptr; }

    /// <summary>
    /// Creates a gl::Texture from this image.
    /// </summary>
    /// <returns>gl::Texture holding this image</returns>
    gl::Texture toTexture() const {
      gl::Texture::Size size{dimensions.x, dimensions.y};
      return gl::Texture(size, gl::Texture::formatFromChannels(channels),
                         gl::Texture::internalFormatFromChannels(channels),
                         data);
    }

    const glm::ivec2& getDimensions() const { return dimensions; }
    const int getChannels() const { return channels; }
    /// <summary>
    /// Gets a pointer to the image data.
    /// To be used when creating a texture, should not be manipulated or freed.
    /// </summary>
    /// <returns>Pointer to the pixel data</returns>
    const unsigned char* getData() const { return data; }

  protected:
    glm::ivec2 dimensions;
    int channels;
    unsigned char* data;
  };
} // namespace engine