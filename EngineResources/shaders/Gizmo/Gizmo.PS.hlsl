// 頂点色をそのまま出す。ライティングもテクスチャも使わない.
struct PixelShaderInput {
	float32_t4 position : SV_POSITION;
	float32_t4 color : COLOR0;
};

float32_t4 main(PixelShaderInput input) : SV_TARGET {
	return input.color;
}
