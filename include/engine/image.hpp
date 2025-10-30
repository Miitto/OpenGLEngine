#pragma once

#include <expected>
#include <gl/texture.hpp>
#include <glm/glm.hpp>
#include <stb/image.h>
#include <string_view>

namespace engine {
  class Image {
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

    static inline std::expected<Image, std::string>
    fromFile(std::string_view file, int desiredChannels = 0) {
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

    gl::Texture toTexture() const {
      gl::Texture::Size size{dimensions.x, dimensions.y};
      return gl::Texture(size, gl::Texture::formatFromChannels(channels),
                         gl::Texture::internalFormatFromChannels(channels),
                         data);
    }

  protected:
    glm::ivec2 dimensions;
    int channels;
    unsigned char* data;
  };
} // namespace engine