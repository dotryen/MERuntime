//
// Created by ryen on 2/24/25.
//

#pragma once

#include <vector>
#include <MECore/render/Window.h>

namespace ME::render {
    class BufferedWindow {
        private:
        void CreateFramebuffers();
        void DestroyFramebuffers();

        public:
        Window* window;
        std::vector<nvrhi::FramebufferHandle> framebuffers;

        BufferedWindow() = default;

        void Initialize() {
            window->beforeRefresh = [this](){ DestroyFramebuffers(); };
            window->afterRefresh = [this](){ CreateFramebuffers(); };
            CreateFramebuffers();
        }

        nvrhi::FramebufferHandle GetFramebuffer() const {
            return framebuffers[window->GetCurrentSwapchainIndex()];
        }
    };
}
