#include "Particle.hlsli"

struct MaterialForGPU {
	float32_t4 color;
	int32_t enableLighting;
	// float32_t4x4 uvTransform; 一時的に削除.
};
ConstantBuffer<MaterialForGPU> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput {
	float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input) {
	PixelShaderOutput output;
	
	// float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
	// float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
	float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
	output.color.rgb = gMaterial.color.rgb * textureColor.rgb;
	output.color.a = gMaterial.color.a * textureColor.a;
	
	// output.colorのa値が0のときにPixelを棄却.
	if (output.color.a == 0.0f) {
		discard;
	}
	
	return output;
}
