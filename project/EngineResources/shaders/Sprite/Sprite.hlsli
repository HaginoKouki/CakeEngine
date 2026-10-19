// b0 / VERTEX (RootParam1). Per-sprite data pushed by the renderer for each draw.
struct SpriteForGPU {
	float4x4 wvp;    // world * orthographic projection.
	float4 color;    // Color to multiply.
	float4 uvRect;   // xy = UV origin, zw = UV size.
	float2 size;     // Display size in pixels.
	float2 pivot;    // Reference point on the rectangle (0..1). (0,0) is top-left.
	float2 uvOffset; // UV scroll amount.
	float2 flip;     // 1 = flipped, 0 = not flipped. x = horizontal, y = vertical.
};

struct VSOutput {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
	float4 color : COLOR0;
};