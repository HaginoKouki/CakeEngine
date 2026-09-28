#include "Sprite.hlsli"

ConstantBuffer<SpriteMaterial> gMaterial : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

float4 main(VSOutput input) : SV_TARGET {
	float2 uv = gMaterial.uvRect.xy + input.corner * gMaterial.uvRect.zw;
	float4 color = gTexture.Sample(gSampler, uv) * gMaterial.color;
	if (color.a <= 0.0f) {
		discard;
	}
	return color;
}