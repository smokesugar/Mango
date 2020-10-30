
cbuffer GlobalUniforms : register(b0)
{
    matrix prevViewProjection;
    matrix viewProjection;
};

cbuffer IndividualUniforms : register(b1)
{
    matrix prevModel;
    matrix model;
    float4 color;
}

struct VSOut
{
    float3 color : Color;
    float3 normal : Normal;
    float2 uv : TexCoord;
    float3 vel : Velocity;
    float4 svpos : SV_Position;
};

VSOut main(float3 pos : Position, float3 normal : Normal, float2 uv : TexCoord)
{
    VSOut vso;
	
    vso.normal = mul((float3x3) model, normal);
    vso.uv = uv;
    vso.svpos = mul(viewProjection, mul(model, float4(pos, 1.0f)));
    vso.color = float3(1.0f, 1.0f, 1.0f);
	
    float4 prevPos = mul(prevViewProjection, mul(prevModel, float4(pos, 1.0f)));
    vso.vel = vso.svpos.xyz / vso.svpos.w - prevPos.xyz / prevPos.w;
    vso.vel.y *= -1.0f;
    vso.vel /= 2.0f;
	
    return vso;
}