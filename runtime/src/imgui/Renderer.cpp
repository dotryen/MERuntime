#include <spdlog/spdlog.h>

#include "MERuntime/imgui/Renderer.h"
#include "MERuntime/imgui/Shaders.h"

namespace ME::imgui {
    bool Renderer::Initialize(nvrhi::IDevice* device) {
        this->device = device;
        // Immediate is required on DX11 only? but having it in immediate mode breaks everything for some reason.
        commandList = device->createCommandList();

        // Create shaders
        auto vertexShaderDesc = nvrhi::ShaderDesc(nvrhi::ShaderType::Vertex);
        vertexShaderDesc.debugName = "ImGui Vertex Shader";
        vertexShader = device->createShader(vertexShaderDesc, vertexShaderData, vertexShaderSize);

        auto pixelShaderDesc = nvrhi::ShaderDesc(nvrhi::ShaderType::Pixel);
        pixelShaderDesc.debugName = "ImGui Pixel Shader";
        pixelShader = device->createShader(pixelShaderDesc, pixelShaderData, pixelShaderSize);

        if (!vertexShader || !pixelShader) {
            spdlog::error("Failed to create ImGui shaders");
            return false;
        }

        // Create layout
        nvrhi::VertexAttributeDesc vertexAttributeLayout[] = {
            { "POSITION", nvrhi::Format::RG32_FLOAT, 1, 0, offsetof(ImDrawVert, pos), sizeof(ImDrawVert), false },
            { "TEXCOORD", nvrhi::Format::RG32_FLOAT, 1, 0, offsetof(ImDrawVert, uv), sizeof(ImDrawVert), false },
            { "COLOR", nvrhi::Format::RGBA8_UNORM, 1, 0, offsetof(ImDrawVert, col), sizeof(ImDrawVert), false },
        };
        shaderAttribLayout = device->createInputLayout(vertexAttributeLayout, 3, vertexShader);

        // Create pipeline descriptor
        nvrhi::BlendState blendState;
        blendState.targets[0].setBlendEnable(true)
            .setSrcBlend(nvrhi::BlendFactor::SrcAlpha)
            .setDestBlend(nvrhi::BlendFactor::InvSrcAlpha)
            .setSrcBlendAlpha(nvrhi::BlendFactor::InvSrcAlpha)
            .setDestBlendAlpha(nvrhi::BlendFactor::Zero);

        auto rasterState = nvrhi::RasterState()
            .setFillSolid()
            .setCullNone()
            .setScissorEnable(true)
            .setDepthClipEnable(true);

        auto depthStencilState = nvrhi::DepthStencilState()
            .disableDepthTest()
            .enableDepthWrite()
            .disableStencil()
            .setDepthFunc(nvrhi::ComparisonFunc::Always);

        nvrhi::RenderState renderState;
        renderState.blendState = blendState;
        renderState.depthStencilState = depthStencilState;
        renderState.rasterState = rasterState;

        nvrhi::BindingLayoutDesc layoutDesc;
        layoutDesc.visibility = nvrhi::ShaderType::All;
        layoutDesc.bindings = {
            nvrhi::BindingLayoutItem::PushConstants(0, sizeof(float) * 2),
            nvrhi::BindingLayoutItem::Texture_SRV(0),
            nvrhi::BindingLayoutItem::Sampler(0)
        };
        bindingLayout = device->createBindingLayout(layoutDesc);

        pipelineDesc.primType = nvrhi::PrimitiveType::TriangleList;
        pipelineDesc.inputLayout = shaderAttribLayout;
        pipelineDesc.VS = vertexShader;
        pipelineDesc.PS = pixelShader;
        pipelineDesc.renderState = renderState;
        pipelineDesc.bindingLayouts = { bindingLayout };

        // Create font sampler
        const auto samplerDesc = nvrhi::SamplerDesc()
            .setAllAddressModes(nvrhi::SamplerAddressMode::Wrap)
            .setAllFilters(true);

        fontSampler = device->createSampler(samplerDesc);
        if (fontSampler == nullptr) {
            spdlog::error("Failed to create ImGui font sampler");
            return false;
        }

        return true;
    }

    bool Renderer::UpdateFontTexture() {
        ImGuiIO& io = ImGui::GetIO();

        if (fontTexture && io.Fonts->TexID) return true;

        unsigned char* pixels;
        int width, height;

        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        if (!pixels) return false;

        nvrhi::TextureDesc desc;
        desc.width = width;
        desc.height = height;
        desc.format = nvrhi::Format::RGBA8_UNORM;
        desc.debugName = "ImGui font texture";

        fontTexture = device->createTexture(desc);
        if (fontTexture == nullptr) return false;

        commandList->open();

        commandList->beginTrackingTextureState(fontTexture, nvrhi::AllSubresources, nvrhi::ResourceStates::Common);
        commandList->writeTexture(fontTexture, 0, 0, pixels, width * 4);
        commandList->setPermanentTextureState(fontTexture, nvrhi::ResourceStates::ShaderResource);

        commandList->commitBarriers();
        commandList->close();
        device->executeCommandList(commandList);

        io.Fonts->TexID = reinterpret_cast<uint64_t>(fontTexture.Get());

        return true;
    }

    bool Renderer::Render(nvrhi::IFramebuffer* framebuffer) {
        ImDrawData* drawData = ImGui::GetDrawData();
        const auto& io = ImGui::GetIO();

        commandList->open();
        commandList->beginMarker("ImGui");

        if (!UpdateGeometry(commandList)) {
            commandList->close();
            return false;
        }

        drawData->ScaleClipRects(io.DisplayFramebufferScale);
        float invDisplaySize[2] = { 1.0f / io.DisplaySize.x, 1.0f / io.DisplaySize.y };

        nvrhi::GraphicsState drawState;
        drawState.framebuffer = framebuffer;
        assert(drawState.framebuffer);

        drawState.pipeline = GetPipeline(framebuffer);
        drawState.viewport.viewports.push_back(nvrhi::Viewport(io.DisplaySize.x * io.DisplayFramebufferScale.x, io.DisplaySize.y * io.DisplayFramebufferScale.y));
        drawState.viewport.scissorRects.resize(1);

        nvrhi::VertexBufferBinding vertexBufferBinding;
        vertexBufferBinding.buffer = gpuVertexBuffer;
        vertexBufferBinding.slot = 0;
        vertexBufferBinding.offset = 0;
        drawState.vertexBuffers.push_back(vertexBufferBinding);

        drawState.indexBuffer.buffer = gpuIndexBuffer;
        drawState.indexBuffer.format = (sizeof(ImDrawIdx) == 2 ? nvrhi::Format::R16_UINT : nvrhi::Format::R32_UINT);
        drawState.indexBuffer.offset = 0;

        int vertexOffset = 0;
        int indexOffset = 0;
        for(int n = 0; n < drawData->CmdListsCount; n++) {
            const ImDrawList* cmdList = drawData->CmdLists[n];
            for(int i = 0; i < cmdList->CmdBuffer.Size; i++) {
                const ImDrawCmd* drawCmd = &cmdList->CmdBuffer[i];

                if (drawCmd->UserCallback) {
                    drawCmd->UserCallback(cmdList, drawCmd);
                } else {
                    drawState.bindings = { GetBindingSet((nvrhi::ITexture*)drawCmd->TextureId) };
                    assert(drawState.bindings[0]);

                    drawState.viewport.scissorRects[0] = nvrhi::Rect(int(drawCmd->ClipRect.x),
                                                                     int(drawCmd->ClipRect.z),
                                                                     int(drawCmd->ClipRect.y),
                                                                     int(drawCmd->ClipRect.w));

                    nvrhi::DrawArguments drawArguments;
                    drawArguments.vertexCount = drawCmd->ElemCount;
                    drawArguments.startIndexLocation = indexOffset;
                    drawArguments.startVertexLocation = vertexOffset;

                    commandList->setGraphicsState(drawState);
                    commandList->setPushConstants(invDisplaySize, sizeof(invDisplaySize));
                    commandList->drawIndexed(drawArguments);
                }
                indexOffset += drawCmd->ElemCount;
            }
            vertexOffset += cmdList->VtxBuffer.Size;
        }

        commandList->endMarker();
        commandList->close();
        device->executeCommandList(commandList);

        return true;
    }

    bool Renderer::UpdateGeometry(nvrhi::ICommandList* commandList) {
        ImDrawData* drawData = ImGui::GetDrawData();

        auto vertCount = drawData->TotalVtxCount;
        if (!ReallocateBuffer(gpuVertexBuffer, vertCount * sizeof(ImDrawVert), (vertCount + 5000) * sizeof(ImDrawVert), false)) {
            return false;
        }

        auto indexCount = drawData->TotalIdxCount;
        if (!ReallocateBuffer(gpuIndexBuffer, indexCount * sizeof(ImDrawIdx), (indexCount + 5000) * sizeof(ImDrawIdx), true)) {
            return false;
        }

        cpuVertexBuffer.resize(gpuVertexBuffer->getDesc().byteSize / sizeof(ImDrawVert));
        cpuIndexBuffer.resize(gpuIndexBuffer->getDesc().byteSize / sizeof(ImDrawIdx));

        ImDrawVert* vertexArr = &cpuVertexBuffer[0];
        ImDrawIdx* indexArr = &cpuIndexBuffer[0];

        for (int i = 0; i < drawData->CmdListsCount; i++) {
            const ImDrawList* cmdList = drawData->CmdLists[i];

            memcpy(vertexArr, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(indexArr, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));

            vertexArr += cmdList->VtxBuffer.Size;
            indexArr += cmdList->IdxBuffer.Size;
        }

        commandList->writeBuffer(gpuVertexBuffer, &cpuVertexBuffer[0], gpuVertexBuffer->getDesc().byteSize);
        commandList->writeBuffer(gpuIndexBuffer, &cpuIndexBuffer[0], gpuIndexBuffer->getDesc().byteSize);

        return true;
    }

    bool Renderer::ReallocateBuffer(nvrhi::BufferHandle& buffer, size_t requiredSize, size_t reallocSize, const bool indexBuffer) {
        if (buffer == nullptr || size_t(buffer->getDesc().byteSize) < requiredSize) {
            nvrhi::BufferDesc desc;
            desc.byteSize = reallocSize;
            desc.structStride = 0;
            desc.debugName = indexBuffer ? "ImGui Index Buffer" : "ImGui Vertex Buffer";
            desc.canHaveUAVs = false;
            desc.isVertexBuffer = !indexBuffer;
            desc.isIndexBuffer = indexBuffer;
            desc.isDrawIndirectArgs = false;
            desc.isVolatile = false;
            desc.initialState = indexBuffer ? nvrhi::ResourceStates::IndexBuffer : nvrhi::ResourceStates::VertexBuffer;
            desc.keepInitialState = true;

            buffer = device->createBuffer(desc);
            if (!buffer) return false;
        }
        return true;
    }

    nvrhi::IGraphicsPipeline* Renderer::GetPipeline(nvrhi::IFramebuffer* fb) {
        if (pipeline) return pipeline;

        pipeline = device->createGraphicsPipeline(pipelineDesc, fb);
        assert(pipeline);

        return pipeline;
    }

    nvrhi::IBindingSet* Renderer::GetBindingSet(nvrhi::ITexture* texture) {
        auto iter = bindingsCache.find(texture);
        if (iter != bindingsCache.end()) return iter->second;

        nvrhi::BindingSetDesc desc;

        desc.bindings = {
            nvrhi::BindingSetItem::PushConstants(0, sizeof(float) * 2),
            nvrhi::BindingSetItem::Texture_SRV(0, texture),
            nvrhi::BindingSetItem::Sampler(0, fontSampler)
        };

        auto binding = device->createBindingSet(desc, bindingLayout);
        assert(binding);

        bindingsCache[texture] = binding;
        return binding;
    }

}