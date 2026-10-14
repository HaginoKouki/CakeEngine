#include "Particle.hlsli"

struct MaterialForGPU {
	float32_t4 color;
	int32_t enableLighting;
};
ConstantBuffer<MaterialForGPU> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput {
	float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input) {
	PixelShaderOutput output;
	
	float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
	output.color.rgb = gMaterial.color.rgb * textureColor.rgb * input.color.rgb;
	output.color.a = gMaterial.color.a * textureColor.a * input.color.a;
	
	// output.colorのa値が0のときにPixelを棄却.
	if (output.color.a == 0.0f) {
		discard;
	}
	
	return output;
}
