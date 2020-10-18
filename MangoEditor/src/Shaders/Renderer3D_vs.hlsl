
cbuffer GlobalUniforms : register(b0) {
	row_major matrix viewProjection;
};

cbuffer IndividualUniforms : register(b1) {
	row_major matrix model;
	float4 color;
}

struct VSOut {
	float3 pos : Position;
	float3 normal : Normal;
	float2 uv : TexCoord;
	float4 svpos : SV_Position;
};

VSOut main(float3 pos : Position, float3 normal : Normal, float2 uv : TexCoord) {
	VSOut vso;
	vso.pos = mul(float4(pos, 1.0f), model).xyz;
	vso.normal = mul(normal, (float3x3)model);
	vso.uv = uv;
	vso.svpos = mul(float4(vso.pos, 1.0f), viewProjection);
	return vso;
}