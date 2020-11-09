
struct VSOut
{
    float2 uv : TexCoord;
    float4 svpos : SV_Position;
};

VSOut main(float3 pos : Position, float2 uv : TexCoord)
{
    VSOut vso;
    vso.uv = uv;
    vso.svpos = float4(pos, 1.0);
    return vso;
}