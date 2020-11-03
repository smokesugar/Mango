
cbuffer Uniforms : register(b1)
{
    matrix prevModel;
    matrix mvp;
}

struct VSOut
{
    float depth : Depth;
    float4 svpos : SV_Position;
};

VSOut main(float3 pos : Position, float3 normal : Normal, float2 uv : TexCoord)
{
    VSOut vso;
    vso.svpos = mul(mvp, float4(pos, 1.0f));
    vso.depth = vso.svpos.z / vso.svpos.w;
    return vso;
}