
struct VSOut {
	float3 pos : Position;
	float3 normal : Normal;
	float2 uv : TexCoord;
	float4 svpos : SV_Position;
};

float4 main(VSOut vso) : SV_Target{
	float3 N = normalize(vso.normal);
	float3 L = normalize(float3(1.0f, 2.0f, -2.0f));
	float dif = max(dot(N, L), 0.01f);
	return float4(float3(dif, dif, dif), 1.0f);
}