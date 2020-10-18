
struct VSOut {
	float3 pos : Position;
	float3 normal : Normal;
	float2 uv : TexCoord;
	float4 svpos : SV_Position;
};

float4 main(VSOut vso) : SV_Target{
	return float4(vso.normal * 0.5f + 0.5f, 1.0f);
}