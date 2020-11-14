
cbuffer Uniforms : register(b0)
{
    matrix MVP;
    float3 color;
    float padding;
}

struct VSOut
{
    float3 color : Color;
    float4 svpos : SV_Position;
};

VSOut main(float3 pos : Position, float3 norm : Normal, float2 uv : TexCoord)
{
    VSOut vso;
    vso.svpos = mul(MVP, float4(pos, 1.0f));
    vso.color = color;
    return vso;
}