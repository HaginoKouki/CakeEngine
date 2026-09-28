// Panoramic.PS.hlsl
cbuffer SkyboxMatrix : register(b0) {
    float4x4 gInvViewProjRotationOnly;
};

Texture2D<float4> gPanoramaTexture : register(t0);
SamplerState gSampler : register(s0);

static const float kPi = 3.14159265f;

float4 main(float4 position : SV_Position, float2 clipXY : TEXCOORD0) : SV_TARGET {
    float4 worldDir4 = mul(float4(clipXY, 1.0f, 1.0f), gInvViewProjRotationOnly);
    float3 dir = normalize(worldDir4.xyz);

    float phi = atan2(dir.z, dir.x);
    float theta = asin(clamp(dir.y, -1.0f, 1.0f));

    float u = 0.5f - phi / (2.0f * kPi);
    float v = 0.5f - theta / kPi;

    return gPanoramaTexture.Sample(gSampler, float2(u, v));
}
