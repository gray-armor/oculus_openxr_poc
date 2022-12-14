#include "pch.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>
#include <boost/system/errc.hpp>

#include "log.h"
#include "platform_data.h"
#include "platform_plugin.h"
#include "graphics_plugin.h"
#include "program.h"

void android_main(struct android_app *app) {
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket socket(io_service);

    const char *address = "192.168.0.5";
    const int port = 51111;

    boost::system::error_code error;
    socket.connect(
            boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(address), port),
            error);

    if (error) {
        LOG_ERROR("Failed to connect");
    } else {
        LOG_INFO("Connection OK");
        const std::string msg = "ping!";
        boost::system::error_code e;
        boost::asio::write(socket, boost::asio::buffer(msg), e);
    }

    try {
        JNIEnv *Env;
        app->activity->vm->AttachCurrentThread(&Env, nullptr);

        std::shared_ptr <PlatformData> data = std::make_shared<PlatformData>();
        data->applicationVM = app->activity->vm;
        data->applicationActivity = app->activity->clazz;

        bool requestRestart = false;
        bool exitRenderLoop = false;

        std::shared_ptr <IPlatformPlugin> platformPlugin = CreatePlatformPlugin(data);
        std::shared_ptr <IGraphicsPlugin> graphicsPlugin = CreateGraphicsPlugin();

        std::shared_ptr <IOpenXrProgram> program = CreateOpenXrProgram(platformPlugin,
                                                                       graphicsPlugin);

        PFN_xrInitializeLoaderKHR initializeLoader = nullptr;
        if (XR_SUCCEEDED(xrGetInstanceProcAddr(XR_NULL_HANDLE,
                                               "xrInitializeLoaderKHR",
                                               (PFN_xrVoidFunction * )(&initializeLoader)))) {
            XrLoaderInitInfoAndroidKHR loaderInitInfoAndroid;
            memset(&loaderInitInfoAndroid, 0, sizeof(loaderInitInfoAndroid));
            loaderInitInfoAndroid.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
            loaderInitInfoAndroid.next = nullptr;
            loaderInitInfoAndroid.applicationVM = app->activity->vm;
            loaderInitInfoAndroid.applicationContext = app->activity->clazz;
            initializeLoader((const XrLoaderInitInfoBaseHeaderKHR *) &loaderInitInfoAndroid);
        }

        program->CreateInstance();
        program->InitializeSystem();

        XrEnvironmentBlendMode blendMode = program->GetPreferredBlendMode();
        graphicsPlugin->SetBlendMode(blendMode);

        program->InitializeDevice();
        program->InitializeSession();
        program->CreateSwapchains();

        while (!app->destroyRequested) {
            for (;;) {
                int events;
                struct android_poll_source *source;

                if (ALooper_pollAll(0, nullptr, &events, (void **) &source) < 0) {
                    break;
                }

                if (source != nullptr) {
                    source->process(app, source);
                }
            }

            program->PollEvents(&exitRenderLoop, &requestRestart);
            if (exitRenderLoop) {
                break;
            }

            if (!program->IsSessionRunning()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
                continue;
            }

            program->PollActions();
            program->RenderFrame();
        }

        app->activity->vm->DetachCurrentThread();
    } catch (const std::exception &ex) {
        LOG_ERROR("%s", ex.what());
    } catch (...) {
        LOG_ERROR("Unknown Error");
    }
}