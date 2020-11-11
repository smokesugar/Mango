
cbuffer Transforms : register(b0)
{
    matrix viewMatrix;
    matrix projectionMatrix;
    float environmentStrength;
}

struct VSOut
{
    float3 pos : Position;
    float environmentStrength : EnvStrength;
    float4 svpos : SV_Position;
};
 
VSOut main(float3 pos : Position, float3 norm : Normal, float2 uv : TexCoord)
{
    VSOut vso;
    vso.pos = pos;
    vso.environmentStrength = environmentStrength;
    float3 viewPos = mul((float3x3) viewMatrix, pos);
    vso.svpos = mul(projectionMatrix, float4(viewPos, 1.0f));
    return vso;
}