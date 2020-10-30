struct VSOut
{
    float3 color : Color;
    float3 normal : Normal;
    float2 uv : TexCoord;
    float3 vel : Velocity;
    float4 svpos : SV_Position;
};

struct PSOut
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
    float4 velocity : SV_Target2;
};

PSOut main(VSOut vso)
{
    PSOut pso;
    pso.color.rgb = vso.color;
    pso.normal.xyz = vso.normal;
    pso.velocity = float4(vso.vel, 1.0);
    return pso;
}

