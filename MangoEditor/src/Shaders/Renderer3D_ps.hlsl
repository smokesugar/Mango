
struct VSOut {
	float3 pos : Position;
	float3 normal : Normal;
	float2 uv : TexCoord;
    float3 vel : Velocity;
	float4 svpos : SV_Position;
};

struct PSOut
{
    float4 color : SV_Target0;
    float4 velocity : SV_Target1;
};

PSOut main(VSOut vso)
{
    PSOut pso;
    float3 N = normalize(vso.normal);
    float3 L = normalize(float3(1.0f, 2.0f, -2.0f));
    float dif = max(dot(N, L), 0.01f);
    pso.color = float4(float3(dif, dif, dif), 1.0f);
    pso.velocity = float4(vso.vel, 1.0);
    return pso;
}