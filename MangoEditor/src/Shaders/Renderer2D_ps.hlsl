
struct VSOut
{
    float3 pos : Position;
    float2 uv : TexCoord;
    float4 col : Color;
    float3 vel : Velocity;
    float4 svpos : SV_Position;
};

Texture2D tex0 : register(t0);
SamplerState sampler0 : register(s0);

struct PSOut
{
    float4 color : SV_Target0;
    float4 velocity : SV_Target1;
};

PSOut main(VSOut vso)
{
    PSOut pso;
    pso.color = tex0.Sample(sampler0, vso.uv) * vso.col;
    pso.velocity = float4(vso.vel, 1.0f);
    return pso;
}