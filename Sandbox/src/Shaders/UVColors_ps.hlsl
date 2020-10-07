
struct VSOut {
	float3 pos : Position;
	float4 svpos : SV_Position;
};

float4 main(VSOut vso) : SV_Target{
	return float4(vso.pos*0.5f+0.5f, 1.0f);
}