#include "Sprite.hlsli"

ConstantBuffer<SpriteTransform> gTransform : register(b0);

static const float2 kCorners[6] = {
	float2(0.0f, 0.0f), float2(1.0f, 0.0f), float2(0.0f, 1.0f), //Top left, top right, bottom left.
	float2(0.0f, 1.0f), float2(1.0f, 0.0f), float2(1.0f, 1.0f), //Bottom left, top right, bottom right.
};

VSOutput main(uint vertexId : SV_VertexID) {
	VSOutput output;
	float2 corner = kCorners[vertexId];

	// Shift the reference point (offset) to the origin and multiply by the size in screen-relative units.
	float2 local = (corner - gTransform.offset) * gTransform.size;

	// Rotation. Rotating directly in screen-aspect-ratio units causes distortion, so convert to an isotropic space before rotating.
	local.y /= gTransform.aspect;
	float s = sin(gTransform.rotation);
	float c = cos(gTransform.rotation);
	local = float2(local.x * c - local.y * s, local.x * s + local.y * c);
	local.y *= gTransform.aspect;

	// Screen coordinates (origin at top-left, 0..1) -> NDC.
	float2 screen = gTransform.position + local;
	output.position = float4(screen.x * 2.0f - 1.0f, 1.0f - screen.y * 2.0f, 0.0f, 1.0f);
	output.corner = corner;
	return output;
}