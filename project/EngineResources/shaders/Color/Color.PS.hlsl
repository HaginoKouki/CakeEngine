#include "Color.hlsli"

struct Parameter {
	float32_t4 color;
};
ConstantBuffer<Parameter> gMaterial : register(b0);

struct PixelShaderOutput {
	float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input) {
	PixelShaderOutput output;
	output.color = gMaterial.color;
	return output;
}
