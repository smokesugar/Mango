
cbuffer Transforms : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
}

struct VSOut
{
    float3 pos : Position;
    float4 svpos : SV_Position;
};
 
VSOut main(float3 pos : Position, float3 norm : Normal, float2 uv : TexCoord)
{
    VSOut vso;
    vso.pos = pos;
    float3 viewPos = mul((float3x3)ViewMatrix, pos);
    vso.svpos = mul(ProjectionMatrix, float4(viewPos, 1.0f));
    return vso;
}