#include "pch.h"
#include "platformdata.h"
#include "platformplugin.h"

namespace {
struct AndroidPlatformPlugin : public IPlatformPlugin {
  AndroidPlatformPlugin(const std::shared_ptr <PlatformData> &data) {
    instanceCreateInfoAndroid = {XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR};
    instanceCreateInfoAndroid.applicationVM = data->applicationVM;
    instanceCreateInfoAndroid.applicationActivity = data->applicationActivity;
  }

  std::vector <std::string> GetInstanceExtensions() const override {
    return {XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME};
  }

  XrBaseInStructure *GetInstanceCreateExtension() const override {
    return (XrBaseInStructure * ) & instanceCreateInfoAndroid;
  }

  XrInstanceCreateInfoAndroidKHR instanceCreateInfoAndroid;
};
}  // namespace

std::shared_ptr <IPlatformPlugin> CreatePlatformPlugin(const std::shared_ptr <PlatformData> &data) {
  return std::make_shared<AndroidPlatformPlugin>(data);
}