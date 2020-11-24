
Texture2D aoTexture : register(t0);
Texture2D litImage : register(t1);

SamplerState sampler0 : register(s0);

cbuffer constantBuffer : register(b0)
{
    matrix view;
    matrix projection;
    float4 perspectiveValues;
    float randomSeed;
    float radius;
    float2 padding;
};

struct VSOut
{
    float2 uv : TexCoord;
    float4 svpos : SV_Position;
};

float3 GetViewPosition(float2 uv, float linearDepth)
{
    float2 clipSpace = uv * 2.0f - 1.0f;
    clipSpace.y *= -1.0f;
    
    float3 pos;
    pos.xy = clipSpace * perspectiveValues.xy * linearDepth;
    pos.z = linearDepth;
    
    return pos;
}

float LinearizeDepth(float depth)
{
    float linearDepth = perspectiveValues.z / (depth + perspectiveValues.w);
    return linearDepth;
}

float4 main(VSOut vso) : SV_Target
{
    float occlusion = aoTexture.Sample(sampler0, vso.uv).r;
    
    return litImage.Sample(sampler0, vso.uv) * occlusion;
}