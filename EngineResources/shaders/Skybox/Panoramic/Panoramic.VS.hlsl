struct VSOutput {
    float4 position : SV_Position;
    float2 clipXY   : TEXCOORD0;
};

VSOutput main(uint vertexId : SV_VertexID) {
    VSOutput output;
    float2 ndc = float2(
        (vertexId == 1) ? 3.0f : -1.0f,
        (vertexId == 2) ? 3.0f : -1.0f
    );
    output.clipXY = ndc;
    output.position = float4(ndc, 1.0f, 1.0f);
    return output;
}