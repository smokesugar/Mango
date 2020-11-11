
struct VSOut {
	float3 pos : Position;
	float2 uv : TexCoord;
	float4 col : Color;
	float4 svpos : SV_Position;
};

Texture tex0 : register(t0);
SamplerState sampler0 : register(s0);

float4 main(VSOut vso) : SV_Target{
	return tex0.Sample(sampler0, vso.uv) * vso.col;
}