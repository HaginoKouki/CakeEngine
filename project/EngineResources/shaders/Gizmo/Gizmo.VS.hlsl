// ギズモ（デバッグ線）用の頂点シェーダー.
// 頂点は既にワールド空間なので、viewProjection を1回掛けるだけ.
// b0(VERTEX) は RootSignature のパラメータ1。Renderer が viewProj を積む.

struct CameraMatrix {
	float32_t4x4 viewProjection;
};
ConstantBuffer<CameraMatrix> gCamera : register(b0);

struct VertexShaderInput {
	float32_t4 position : POSITION0;
	float32_t4 color : COLOR0;
};

struct VertexShaderOutput {
	float32_t4 position : SV_POSITION;
	float32_t4 color : COLOR0;
};

VertexShaderOutput main(VertexShaderInput input) {
	VertexShaderOutput output;
	// mul の並びは既存の Standard.VS.hlsl と必ず同じにすること.
	// 逆にすると行優先／列優先の解釈がずれて何も映らなくなる.
	output.position = mul(input.position, gCamera.viewProjection);
	output.color = input.color;
	return output;
}
