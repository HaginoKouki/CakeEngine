#include "Standard.hlsli"

struct Parameter {
	float32_t4 color;
	float32_t2 tiling;
	float32_t2 offset;
	int lightingType;
};
ConstantBuffer<Parameter> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct DirectionalLight {
	float32_t4 color;
	float32_t3 direction;
	float intensity;
};
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

struct PixelShaderOutput {
	float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input) {
	PixelShaderOutput output;
	
	// UV変換.
	float2 uv = input.texcoord * gMaterial.tiling + gMaterial.offset;
	float32_t4 textureColor = gTexture.Sample(gSampler, uv);

	// textureColorのa値が0の時にPixelを棄却.
	if (textureColor.a == 0.0) {
		discard;
	}

	if (gMaterial.lightingType == 1) {
		// half lambert.
		float NdoL = dot(normalize(input.normal), -gDirectionalLight.direction);
		float cos = pow(NdoL * 0.5f + 0.5f, 2.0f);
		output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
		output.color.a = gMaterial.color.a * textureColor.a;
	} else if (gMaterial.lightingType == 2) {
		// lambert.
		float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
		output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
		output.color.a = gMaterial.color.a * textureColor.a;
	} else {
		output.color.rgb = gMaterial.color.rgb * textureColor.rgb;
		output.color.a = gMaterial.color.a * textureColor.a;
	}
	
	// output.colorのa値が0のときにPixelを棄却.
	if (output.color.a == 0.0f) {
		discard;
	}
	
	return output;
}
