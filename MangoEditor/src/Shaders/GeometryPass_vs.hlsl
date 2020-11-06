
cbuffer TransformUniforms : register(b0)
{
    matrix transform;
    matrix prevMVP;
    matrix MVP;
};

struct VSOut
{
    float4 pos : Position;
    float4 posSS : ScreenSpacePosition;
    float4 prevPos : PreviousScreenSpacePosition;
    float3 normal : Normal;
    float2 uv : TexCoord;
    float4 svpos : SV_Position;
};

VSOut main(float3 pos : Position, float3 normal : Normal, float2 uv : TexCoord)
{
    VSOut vso;
	
    vso.normal = mul((float3x3) transform, normal);
    vso.uv = uv;
    vso.pos = mul(transform, float4(pos, 1.0f));
    vso.svpos = mul(MVP, float4(pos, 1.0f));
    vso.posSS = vso.svpos;
    vso.prevPos = mul(prevMVP, float4(pos, 1.0f));
	
    return vso;
}