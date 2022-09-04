#include "pch.h"

#include <exception>
#include <memory>

#include "graphicsplugin.h"
#include "platformdata.h"
#include "platformplugin.h"
#include "program.h"
#include "log.h"

void android_main(struct android_app *app) {
  try {
    JNIEnv *Env;
    app->activity->vm->AttachCurrentThread(&Env, nullptr);

    auto data = std::make_shared<PlatformData>();
    data->applicationVM = app->activity->vm;
    data->applicationActivity = app->activity->clazz;

    auto platformPlugin = CreatePlatformPlugin(data);
    auto graphicsPlugin = CreateGraphicsPlugin();
    auto program = CreateOpenXrProgram(graphicsPlugin, platformPlugin);

    PFN_xrInitializeLoaderKHR initializeLoader = nullptr;
    if (XR_SUCCEEDED(xrGetInstanceProcAddr(XR_NULL_HANDLE,
                                           "xrInitializeLoaderKHR",
                                           (PFN_xrVoidFunction *) (&initializeLoader)))) {
      XrLoaderInitInfoAndroidKHR loaderInitInfoAndroid;
      memset(&loaderInitInfoAndroid, 0, sizeof(loaderInitInfoAndroid));
      loaderInitInfoAndroid.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
      loaderInitInfoAndroid.next = nullptr;
      loaderInitInfoAndroid.applicationVM = app->activity->vm;
      loaderInitInfoAndroid.applicationContext = app->activity->clazz;
      initializeLoader((const XrLoaderInitInfoBaseHeaderKHR*)&loaderInitInfoAndroid);
    }

    program->CreateInstance();

    while(true) {
      for (;;) {
        int events;
        struct android_poll_source *source;

        if (ALooper_pollAll(0, nullptr, &events, (void**)&source) < 0) {
          break;
        }

        if (source != nullptr) {
          source->process(app, source);
        }
      }
    }
  } catch (const std::exception &ex) {
    LOG_ERROR("%s", ex.what());
  } catch (...) {
    LOG_ERROR("Unknown Error");
  }
}