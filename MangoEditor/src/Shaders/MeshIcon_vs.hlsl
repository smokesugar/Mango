
struct VSOut
{
    float3 norm : Normal;
    float4 svpos : SV_Position;
};

cbuffer transform
{
    matrix transform;
    matrix MVP;
};

VSOut main(float3 pos : Position, float3 norm : Normal, float2 uv : TexCoord)
{
    VSOut vso;
    vso.norm = mul((float3x3) transform, norm);
    vso.svpos = mul(MVP, float4(pos, 1.0f));
    return vso;
}