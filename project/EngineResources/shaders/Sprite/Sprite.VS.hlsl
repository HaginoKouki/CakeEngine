#include "Sprite.hlsli"

ConstantBuffer<SpriteForGPU> gSprite : register(b0);

static const float2 kCorners[6] = {
	float2(0.0f, 0.0f), float2(1.0f, 0.0f), float2(0.0f, 1.0f), //Top left, top right, bottom left.
	float2(0.0f, 1.0f), float2(1.0f, 0.0f), float2(1.0f, 1.0f), //Bottom left, top right, bottom right.
};

VSOutput main(uint vertexId : SV_VertexID) {
	VSOutput output;
	float2 corner = kCorners[vertexId];
	
	// Move the pivot to the origin and scale to the display size in pixels.
	float2 local = (corner - gSprite.pivot) * gSprite.size;
	output.position = mul(float4(local, 0.0f, 1.0f), gSprite.wvp);
	output.position.z = 0.0f; // Keep inside the depth range of the orthographic projection.

	// Flip by mirroring the corner used for UV.
	float2 flipped = lerp(corner, 1.0f - corner, gSprite.flip);
	output.texcoord = gSprite.uvRect.xy + flipped * gSprite.uvRect.zw + gSprite.uvOffset;
	output.color = gSprite.color;
	return output;
}