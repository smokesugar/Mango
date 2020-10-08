
struct VSOut {
	float3 pos : Position;
	float4 svpos : SV_Position;
};

cbuffer GlobalUniforms : register(b0) {
	row_major matrix viewProjection;
};

cbuffer IndividualUniforms : register(b1) {
	row_major matrix model;
}

VSOut main(float3 pos : Position) {
	VSOut vso;
	float4 worldPos = mul(float4(pos, 1.0), model);
	vso.pos = worldPos.xyz;
	vso.svpos = mul(worldPos, viewProjection);
	return vso;
}