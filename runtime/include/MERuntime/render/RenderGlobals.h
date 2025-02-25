//
// Created by ryen on 2/24/25.
//

#pragma once

#include <MECore/render/RenderGlobals.h>
#include <MECore/render/RenderInterface.h>

#include "BufferedWindow.h"

namespace ME::render {
    inline RenderInterface* interface;
    inline BufferedWindow window;

    bool Initialize();
    void Shutdown();
}
