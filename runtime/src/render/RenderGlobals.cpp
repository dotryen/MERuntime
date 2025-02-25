//
// Created by ryen on 2/24/25.
//

#include <spdlog/spdlog.h>
#include <MECore/render/VulkanInterface.h>

#include "MERuntime/render/RenderGlobals.h"

namespace ME::render {
    bool Initialize() {
        if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
            spdlog::error("Failed to initialize SDL video");
            return false;
        }

        interface = new VulkanInterface();
        if (!interface->CreateInstance()) return false;
        if (!interface->CreateDevice()) return false;

        auto params = WindowParameters{ "MERuntime", 1280, 720 };
        window.window = interface->CreateWindow(&params);
        window.window->CreateSwapchain();
        window.Initialize();

        mainWindow = window.window;

        return true;
    }

    void Shutdown() {
        interface->DestroyDevice();
        delete interface;
    }
}
