
struct VSOut {
	float3 pos : Position;
	float2 uv : TexCoord;
	float4 col : Color;
	float4 svpos : SV_Position;
};

cbuffer GlobalUniforms : register(b0) {
	row_major matrix viewProjection;
};

cbuffer IndividualUniforms : register(b1) {
	row_major matrix model;
	float4 color;
}

VSOut main(float3 pos : Position, float2 uv : TexCoord) {
	VSOut vso;
	float4 worldPos = mul(float4(pos, 1.0), model);
	vso.pos = worldPos.xyz;
	vso.uv = uv;
	vso.svpos = mul(worldPos, viewProjection);
	vso.col = color;
	return vso;
}