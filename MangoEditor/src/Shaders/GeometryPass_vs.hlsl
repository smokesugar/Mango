
cbuffer GlobalUniforms : register(b0)
{
    matrix prevViewProjection;
    matrix viewProjection;
};

cbuffer IndividualUniforms : register(b1)
{
    matrix prevModel;
    matrix model;
}

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
	
    vso.normal = mul((float3x3) model, normal);
    vso.uv = uv;
    vso.pos = mul(model, float4(pos, 1.0f));
    vso.svpos = mul(viewProjection, vso.pos);
    
    vso.posSS = vso.svpos;
    vso.prevPos = mul(prevViewProjection, mul(prevModel, float4(pos, 1.0f)));
	
    return vso;
}