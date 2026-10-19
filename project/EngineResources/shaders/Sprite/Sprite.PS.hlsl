#include "Sprite.hlsli"
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

float4 main(VSOutput input) : SV_TARGET {
	float4 color = gTexture.Sample(gSampler, input.texcoord) * input.color;
	return color;
}