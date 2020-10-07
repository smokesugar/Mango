
struct VSOut {
	float3 pos : Position;
	float4 svpos : SV_Position;
};

cbuffer ColorBuffer : register(b0) {
	float4 color;
}

float4 main(VSOut vso) : SV_Target{
	return color;
}