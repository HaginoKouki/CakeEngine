// b0 / VERTEX (RootParam1). Placement information pushed by the renderer each frame.
struct SpriteTransform {
	float2 position; // Display position ranging from the top-left (0,0) to the bottom-right (1,1) of the screen.
	float2 size;     // Size relative to the screen aspect ratio.
	float2 offset;   // Reference point for position. Range 0–1 on the image; (0,0) is the top-left corner.
	float rotation;  // Radians.
	float aspect;    // Screen aspect ratio (w/h). Filled by the renderer.
};

// b0 / PIXEL (RootParam0). Material.
struct SpriteMaterial {
	float4 color;  // Color to multiply.
	float4 uvRect; // xy = UV origin, zw = UV size (for atlas slicing).
};

struct VSOutput {
	float4 position : SV_POSITION;
	float2 corner : TEXCOORD0; // Local coordinates within the rectangle (0..1). UV calculation is performed in the PS.
};