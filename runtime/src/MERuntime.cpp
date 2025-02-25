//
// Created by ryen on 2/20/25.
//

#include <SDL3/SDL.h>

#include "MERuntime/MERuntime.h"
#include "MERuntime/render/RenderGlobals.h"

namespace ME {
    bool Runtime_Initialize() {
        if (!Core_Initialize(MECoreSystems::All)) {
            spdlog::critical("MERuntime: Failed to initialize Core");
            return false;
        }
        if (!render::Initialize()) {
            return false;
        }
        return true;
    }

    void Runtime_Shutdown() {
        render::Shutdown();
        Core_Shutdown();
    }
}