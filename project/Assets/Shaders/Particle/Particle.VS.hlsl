#include "Particle.hlsli"

// Renderer.cpp の ParticleForGPU と同じ並びにすること.
struct ParticleForGPU {
	float32_t4x4 WVP;
	float32_t4x4 World;
	float32_t4 color;
};
StructuredBuffer<ParticleForGPU> gParticles : register(t0);

struct VertexShaderInput {
	float32_t4 position : POSITION0;
	float32_t2 texcoord : TEXCOORD0;
	float32_t3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input, uint32_t instanceId : SV_InstanceID) {
	VertexShaderOutput output;
	ParticleForGPU particle = gParticles[instanceId];
	output.position = mul(input.position, particle.WVP);
	output.texcoord = input.texcoord;
	output.normal = normalize(mul(input.normal, (float32_t3x3) particle.World));
	output.color = particle.color;

	return output;
}