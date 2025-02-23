#!/bin/sh

dxc -T vs_6_0 -Fo ImGuiVertex.spv -spirv ImGuiVertex.hlsl
dxc -T ps_6_0 -Fo ImGuiPixel.spv -spirv ImGuiPixel.hlsl
