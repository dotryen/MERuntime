#pragma once

#include <unordered_map>
#include <nvrhi/nvrhi.h>
#include <imgui.h>

namespace ME::imgui {
    class Renderer {
        protected:
        nvrhi::DeviceHandle device;
        nvrhi::CommandListHandle commandList;

        nvrhi::ShaderHandle vertexShader;
        nvrhi::ShaderHandle pixelShader;
        nvrhi::InputLayoutHandle shaderAttribLayout;

        nvrhi::TextureHandle fontTexture;
        nvrhi::SamplerHandle fontSampler;

        nvrhi::BufferHandle gpuVertexBuffer;
        nvrhi::BufferHandle gpuIndexBuffer;

        nvrhi::BindingLayoutHandle bindingLayout;
        nvrhi::GraphicsPipelineDesc pipelineDesc;

        nvrhi::GraphicsPipelineHandle pipeline;
        std::unordered_map<nvrhi::ITexture*, nvrhi::BindingSetHandle> bindingsCache;

        std::vector<ImDrawVert> cpuVertexBuffer;
        std::vector<ImDrawIdx> cpuIndexBuffer;

        bool ReallocateBuffer(nvrhi::BufferHandle& buffer, size_t requiredSize, size_t reallocSize, const bool indexBuffer);
        bool UpdateGeometry(nvrhi::ICommandList* commandList);
        nvrhi::IGraphicsPipeline* GetPipeline(nvrhi::IFramebuffer* fb);
        nvrhi::IBindingSet* GetBindingSet(nvrhi::ITexture* texture);

        public:
        bool Initialize(nvrhi::IDevice* device);
        bool UpdateFontTexture();
        bool Render(nvrhi::IFramebuffer* framebuffer);
    };
}
