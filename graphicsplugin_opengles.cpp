#include "pch.h"
#include "check.h"

#include "log.h"
#include "graphicsplugin.h"

struct OpenGLESGraphicsPlugin : public IGraphicsPlugin {
  OpenGLESGraphicsPlugin() {};

  OpenGLESGraphicsPlugin(const OpenGLESGraphicsPlugin &) = delete;
  OpenGLESGraphicsPlugin &operator=(const OpenGLESGraphicsPlugin &) = delete;
  OpenGLESGraphicsPlugin(OpenGLESGraphicsPlugin &&) = delete;
  OpenGLESGraphicsPlugin &operator=(OpenGLESGraphicsPlugin &&) = delete;

  ~OpenGLESGraphicsPlugin() override {}

  std::vector<std::string> GetInstanceExtensions() const override {
    return {XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME};
  }

  void InitializeDevice(XrInstance instance, XrSystemId systemId) override {
  }
};

std::shared_ptr<IGraphicsPlugin>
CreateGraphicsPlugin() {
  return std::make_shared<OpenGLESGraphicsPlugin>();
}
