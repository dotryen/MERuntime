struct PixelInput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

SamplerState texSampler : register(s0);
Texture2D tex : register(t0);

float4 main(PixelInput input) : SV_TARGET {
    return input.color * tex.Sample(texSampler, input.uv);
}
