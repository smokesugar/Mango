
cbuffer Transforms : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
}

struct VSOut
{
    float3 pos : Position;
    float2 uv : TexCoord;
    float4 svpos : SV_Position;
};
 
VSOut main(float3 pos : Position, float3 norm : Normal, float2 uv : TexCoord)
{
    VSOut vso;
    vso.pos = pos;
    float3 worldPos = mul((float3x3)ViewMatrix, pos);
    vso.svpos = mul(ProjectionMatrix, float4(worldPos, 1.0f));
    vso.uv = uv;
    return vso;
}