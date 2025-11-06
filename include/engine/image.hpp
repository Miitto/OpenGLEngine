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
    static bool yFlipped;

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
    fromFile(std::string_view file, bool flipY = false,
             int desiredChannels = 0) {
      if (flipY != yFlipped) {
        stbi_set_flip_vertically_on_load(flipY);
        yFlipped = flipY;
      }

      int width, height, channels;
      unsigned char* data =
          stbi_load(file.data(), &width, &height, &channels, desiredChannels);
      if (data == nullptr) {
        auto err = stbi_failure_reason();
        return std::unexpected(std::string("Failed to load image from ") +
                               file.data() + "\n" + err);
      }
      Image img(glm::ivec2(width, height),
                desiredChannels == 0 ? channels : desiredChannels, data);
      return img;
    }

    ~Image() { stbi_image_free(data); }

    inline bool isValid() const { return data != nullptr; }

    /// <summary>
    /// Creates a gl::Texture from this image.
    /// Set mipmaps to 0 for no mipmaps.
    /// Set mipmaps to >0 to generate that many mipmap levels.
    /// Set mipmaps to -1 to generate full mipmap chain.
    /// </summary>
    /// <returns>gl::Texture holding this image</returns>
    gl::Texture toTexture(int mipmaps = 0) const {
      if (mipmaps == -1) {
        mipmaps = gl::Texture::calcMipLevels(dimensions.x, dimensions.y);
      }
      gl::Texture tex{};
      tex.storage(mipmaps + 1,
                  gl::Texture::internalFormatFromChannels(channels),
                  dimensions);
      tex.subImage(0, 0, 0, dimensions.x, dimensions.y,
                   gl::Texture::formatFromChannels(channels), GL_UNSIGNED_BYTE,
                   data);
      if (mipmaps > 0) {
        tex.generateMipmap();
      }

      return tex;
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