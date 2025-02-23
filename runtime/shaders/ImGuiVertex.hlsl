struct MatrixBuffer {
    float2 invDisplaySize;
};

struct VertInput {
    float2 position : POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

struct VertOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

[[vk::push_constant]]
ConstantBuffer<MatrixBuffer> consts : register(b0);

VertOutput main(VertInput input) {
    VertOutput output;
    output.position.xy = input.position.xy * consts.invDisplaySize * float2(2.0, -2.0) + float2(-1.0, 1.0);
    output.position.zw = float2(0, 1);
    output.uv = input.uv;
    output.color = input.color;
    return output;
}
