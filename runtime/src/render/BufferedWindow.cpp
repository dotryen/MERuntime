//
// Created by ryen on 2/24/25.
//

#include "MERuntime/render/BufferedWindow.h"
#include "MERuntime/render/RenderGlobals.h"

namespace ME::render {
    void BufferedWindow::CreateFramebuffers() {
        auto count = window->GetSwapchainCount();
        framebuffers.resize(count);
        for (int i = 0; i < count ; i++) {
            auto desc = nvrhi::FramebufferDesc().addColorAttachment(window->GetSwapchainTexture(i));
            framebuffers[i] = interface->GetDevice()->createFramebuffer(desc);
        }
    }

    void BufferedWindow::DestroyFramebuffers() {
        framebuffers.clear();
    }
}
