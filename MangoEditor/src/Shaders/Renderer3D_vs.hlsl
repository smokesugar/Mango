
cbuffer GlobalUniforms : register(b0) {
	matrix viewProjection;
};

cbuffer IndividualUniforms : register(b1) {
	matrix model;
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
	vso.pos = mul(model, float4(pos, 1.0f)).xyz;
	vso.normal = mul((float3x3)model, normal);
	vso.uv = uv;
	vso.svpos = mul(viewProjection, float4(vso.pos, 1.0f));
	return vso;
}