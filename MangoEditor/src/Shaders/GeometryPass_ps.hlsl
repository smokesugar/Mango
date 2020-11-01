struct VSOut
{
    float3 pos : Position;
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

Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D roughnessTexture : register(t2);

cbuffer SurfaceData : register(b0)
{
    float3 albedoColor;
    int useNormalMap;
    float roughnessValue;
};

SamplerState sampler0;

float3 GetNormalFromMap(float3 worldPos, float2 uv, float3 normal)
{
    float3 tangentNormal = normalTexture.Sample(sampler0, uv).xyz * 2.0f - 1.0f;

    float3 Q1 = ddx(worldPos);
    float3 Q2 = ddy(worldPos);
    float2 st1 = ddx(uv);
    float2 st2 = ddy(uv);

    float3 N = normalize(normal);
    float3 T = normalize(Q1 * st2.y - Q2 * st1.y);
    float3 B = -normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);

    return normalize(mul(tangentNormal, TBN));
}

PSOut main (VSOut vso)
{
    PSOut pso;
    pso.color.rgb = albedoColor * albedoTexture.Sample(sampler0, vso.uv).rgb;
    pso.color.a = roughnessValue * roughnessTexture.Sample(sampler0, vso.uv).r;
    pso.normal.xyz = useNormalMap ? GetNormalFromMap(vso.pos, vso.uv, vso.normal) : vso.normal;
    pso.velocity = float4(vso.vel, 1.0);
    return pso;
}

