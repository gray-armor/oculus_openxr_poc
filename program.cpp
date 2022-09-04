#include "pch.h"

#include <memory>
#include <vector>

#include "graphicsplugin.h"
#include "platformplugin.h"
#include "program.h"
#include "check.h"
#include "log.h"

inline std::string GetXrVersionString(XrVersion ver) {
  return Fmt("%d.%d.%d", XR_VERSION_MAJOR(ver), XR_VERSION_MINOR(ver), XR_VERSION_PATCH(ver));
}

struct OpenXrProgram : IOpenXrProgram {
  OpenXrProgram(const std::shared_ptr<IGraphicsPlugin> &graphicPlugin,
                const std::shared_ptr<IPlatformPlugin> &platformPlugin) : m_graphicsPlugin(
      graphicPlugin), m_platformPlugin(platformPlugin) {}

  ~OpenXrProgram() {}

  static void LogLayersAndExtensions() {
    const auto logExtensions = [](const char *layerName, int indent = 0) {
      XrResult res;
      uint32_t instanceExtensionCount;

      CHECK_XRCMD(xrEnumerateInstanceExtensionProperties(layerName,
                                                         0,
                                                         &instanceExtensionCount,
                                                         nullptr));

      std::vector<XrExtensionProperties> extensions(instanceExtensionCount);
      for (XrExtensionProperties &extension: extensions) {
        extension.type = XR_TYPE_EXTENSION_PROPERTIES;
      }

      CHECK_XRCMD(xrEnumerateInstanceExtensionProperties(layerName,
                                                         (uint32_t) extensions.size(),
                                                         &instanceExtensionCount,
                                                         extensions.data()));

      const std::string indentStr(indent, ' ');
      LOG_INFO("%sAvailable Extensions: (%d),", indentStr.c_str(), instanceExtensionCount);
      for (const XrExtensionProperties &extension: extensions) {
        LOG_INFO("%s  Name=%s SpecVersion=%d",
                 indentStr.c_str(),
                 extension.extensionName,
                 extension.extensionVersion);
      }
    };

    logExtensions(nullptr);

    {
      uint32_t layerCount;
      CHECK_XRCMD(xrEnumerateApiLayerProperties(0, &layerCount, nullptr));

      std::vector<XrApiLayerProperties> layers(layerCount);
      for (XrApiLayerProperties &layer: layers) {
        layer.type = XR_TYPE_API_LAYER_PROPERTIES;
      }

      CHECK_XRCMD(xrEnumerateApiLayerProperties((uint32_t) layers.size(),
                                                &layerCount,
                                                layers.data()));

      LOG_INFO("Available Layers: (%d)", layerCount);
      for (const XrApiLayerProperties &layer: layers) {
        LOG_INFO("  Name=%s SpecVersion=%s LayerVersion=%d Description=%s",
                 layer.layerName,
                 GetXrVersionString(layer.specVersion).c_str(),
                 layer.layerVersion,
                 layer.description);
        logExtensions(layer.layerName, 4);
      }
    }
  }

  void LogInstanceInfo() {
    CHECK(m_instance != XR_NULL_HANDLE);

    XrInstanceProperties instanceProperties{XR_TYPE_INSTANCE_PROPERTIES};
    CHECK_XRCMD(xrGetInstanceProperties(m_instance, &instanceProperties));

    LOG_INFO("Instance RuntimeName=%s RuntimeVersion=%s", instanceProperties.runtimeName,
             GetXrVersionString(instanceProperties.runtimeVersion).c_str());
  }

  void CreateInstanceInternal() {
    CHECK(m_instance == XR_NULL_HANDLE);

    std::vector<const char *> extensions;

    const std::vector<std::string> platformExtensions = m_platformPlugin->GetInstanceExtensions();
    std::transform(platformExtensions.begin(),
                   platformExtensions.end(),
                   std::back_inserter(extensions),
                   [](const std::string &ext) { return ext.c_str(); });

    const std::vector<std::string> graphicsExtensions = m_graphicsPlugin->GetInstanceExtensions();
    std::transform(graphicsExtensions.begin(),
                   graphicsExtensions.end(),
                   std::back_inserter(extensions),
                   [](const std::string &ext) { return ext.c_str(); });

    XrInstanceCreateInfo createInfo{XR_TYPE_INSTANCE_CREATE_INFO};
    createInfo.next = m_platformPlugin->GetInstanceCreateExtension();
    createInfo.enabledExtensionCount = (uint32_t)extensions.size();
    createInfo.enabledExtensionNames = extensions.data();

    CHECK_XRCMD(xrCreateInstance(&createInfo, &m_instance));
  }

  void CreateInstance() override {
    LogLayersAndExtensions();

    CreateInstanceInternal();

    LogInstanceInfo();
  }

  void InitializeSystem() override {
    CHECK(m_instance != XR_NULL_HANDLE);
    CHECK(m_systemId == XR_NULL_SYSTEM_ID);
    auto formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    XrSystemGetInfo systemInfo{XR_TYPE_SYSTEM_GET_INFO};
    systemInfo.formFactor = formFactor;
    CHECK_XRCMD(xrGetSystem(m_instance, &systemInfo, &m_systemId));

    LOG_INFO("Using system %d for form factor %s", m_systemId, to_string(formFactor));

    CHECK(m_instance != XR_NULL_HANDLE);
    CHECK(m_systemId != XR_NULL_SYSTEM_ID);
  }

 private:
  std::shared_ptr<IGraphicsPlugin> m_graphicsPlugin;
  std::shared_ptr<IPlatformPlugin> m_platformPlugin;
  XrInstance m_instance{XR_NULL_HANDLE};
  XrSystemId m_systemId{XR_NULL_SYSTEM_ID};
};

std::shared_ptr<IOpenXrProgram> CreateOpenXrProgram(const std::shared_ptr<IGraphicsPlugin> &graphicPlugin,
                                                    const std::shared_ptr<IPlatformPlugin> &platformPlugin) {
  return std::make_shared<OpenXrProgram>(graphicPlugin, platformPlugin);
}
