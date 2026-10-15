#include "Object3d.hlsli"

struct MaterialForGPU {
	float32_t4 color;
	int32_t enableLighting;
	float32_t shininess;
	float32_t4 reflectionColor;
};
ConstantBuffer<MaterialForGPU> gMaterial : register(b0);
struct Camera {
	float32_t3 worldPosition;
};
ConstantBuffer<Camera> gCamera : register(b2);
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
	
	// float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
	// float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
	float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);

	// textureColorのa値が0の時にPixelを棄却.
	if (textureColor.a == 0.0) {
		discard;
	}

	float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);

	if (gMaterial.enableLighting != 0) {
		/* half lambert. */
		float NdoL = dot(normalize(input.normal), -gDirectionalLight.direction);
		float cos = pow(NdoL * 0.5f + 0.5f, 2.0f);

		/* Phong */
		// float32_t3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
		// float RdotE = dot(reflectLight, toEye);
		// float specularPow = pow(saturate(RdotE), gMaterial.shininess); // 反射強度.

		/* Blinn-Phong */
		float32_t3 halfVector = normalize(-gDirectionalLight.direction + toEye);
		float NdotL = dot(normalize(input.normal), lightDirection);
		float specularPow = 0.0f;
		if (NdotL > 0.0f) {
			float NDotH = dot(normalize(input.normal), halfVector);
			specularPow = pow(saturate(NDotH), gMaterial.shininess);
		}

		/* 拡散反射. */
		float32_t3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;

		/* 鏡面反射 */
		float32_t3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * gMaterial.reflectionColor.rgb;

		output.color.rgb = diffuse + specular;
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
