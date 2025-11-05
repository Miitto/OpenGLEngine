#include "engine/app.hpp"
#include "engine/glLoader.hpp"
#include "imgui/imgui.h"
#include "logger.hpp"
#include <gl/gl.hpp>

namespace {
  bool engineInitialized = false;

  engine::App::GBuffers createGBuffers(engine::Window::Size size) {
    gl::Texture::Size s = {size.width, size.height};

    auto setParams = [](gl::Texture& tex) {
      tex.setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      tex.setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      tex.setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      tex.setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    };

    engine::App::GBuffers gbuffers;
    gbuffers.diffuse = std::move(gl::Texture());
    gbuffers.diffuse.label("GBuffer Diffuse");
    gbuffers.diffuse.storage(1, GL_RGBA8, s);
    setParams(gbuffers.diffuse);

    gbuffers.normal = std::move(gl::Texture());
    gbuffers.normal.label("GBuffer Normal");
    gbuffers.normal.storage(1, GL_RGBA8, s);
    setParams(gbuffers.normal);

    gbuffers.material = std::move(gl::Texture());
    gbuffers.material.label("GBuffer Material");
    gbuffers.material.storage(1, GL_RGBA8, s);
    setParams(gbuffers.material);

    gbuffers.depthStencil = std::move(gl::Texture());
    gbuffers.depthStencil.label("GBuffer DepthStencil");
    gbuffers.depthStencil.storage(1, GL_DEPTH24_STENCIL8, s);
    setParams(gbuffers.depthStencil);

    gbuffers.fbo = gl::Framebuffer();
    gbuffers.fbo.attachTexture(GL_COLOR_ATTACHMENT0, gbuffers.diffuse);
    gbuffers.fbo.attachTexture(GL_COLOR_ATTACHMENT1, gbuffers.normal);
    gbuffers.fbo.attachTexture(GL_COLOR_ATTACHMENT2, gbuffers.material);
    gbuffers.fbo.attachTexture(GL_DEPTH_STENCIL_ATTACHMENT,
                               gbuffers.depthStencil);

    constexpr GLenum drawBuffers[3] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glNamedFramebufferDrawBuffers(gbuffers.fbo.id(), 3, drawBuffers);

    return gbuffers;
  }
} // namespace

namespace engine {
  bool App::initialized = false;
  std::vector<std::function<void()>> App::pluginShutdowns = {};

  App::App(int width, int height, const char title[])
      : window({width, height, title, true}), input(window), gui(window),
        windowSize(window.size()) {
    if (initialized) {
      Logger::critical("Engine already initialized, exiting");
      throw std::runtime_error("Engine was already initialized");
    }

    auto err = loadPostInitEnginePlugins();
    if (err.has_value()) {
      throw std::runtime_error(err.value());
    }

    gbuffers = std::move(createGBuffers(windowSize));

    initialized = true;
  }

  App::~App() {
    for (auto& shutdown : pluginShutdowns) {
      shutdown();
    }
  }

  void App::onWindowResize(engine::Window::Size newSize) {
    glViewport(0, 0, newSize.width, newSize.height);
    glScissor(0, 0, newSize.width, newSize.height);
    gbuffers = createGBuffers(newSize);
  }

  void App::update(const FrameInfo& frame) {
    window.pollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      gui.sleep(10);
      return;
    }
    gui.newFrame();
    input.imGuiWantsMouse(gui.io().WantCaptureMouse);
    input.imGuiWantsKeyboard(gui.io().WantCaptureKeyboard);

    auto ws = window.size();
    if (ws != windowSize) {
      onWindowResize(ws);
      windowSize = ws;
    }
  }

#define LOAD_PLUGIN(T)                                                         \
  {                                                                            \
    auto res = engine::App::registerPlugin<T>();                               \
    if (res.has_value()) {                                                     \
      return res;                                                              \
    }                                                                          \
  }

  std::optional<std::string> loadPreInitEnginePlugins() {
    if (engineInitialized) {
      Logger::warn("Attempted to load engine plugins multiple times");
      return std::nullopt;
    }

    LOAD_PLUGIN(engine::WindowManager)

    engineInitialized = true;
    // Future engine plugins can be loaded here
    return std::nullopt;
  }

  std::optional<std::string> loadPostInitEnginePlugins() {
    LOAD_PLUGIN(engine::GlLoader)

    return std::nullopt;
  }

  namespace __private {
    void logStart() { engine::Logger::info("Entering main loop"); }
  } // namespace __private
} // namespace engine