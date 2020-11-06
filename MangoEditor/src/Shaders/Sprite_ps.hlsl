
struct VSOut
{
    float4 posSS : ScreenSpacePosition;
    float4 prevPos : PreviousPosition;
    float2 uv : TexCoord;
    float4 col : Color;
    float4 svpos : SV_Position;
};

Texture2D tex0 : register(t0);
SamplerState sampler0 : register(s0);

struct PSOut
{
    float4 color : SV_Target0;
    float4 velocity : SV_Target1;
};

PSOut main(VSOut vso)
{
    PSOut pso;
    pso.color = tex0.Sample(sampler0, vso.uv) * vso.col;
    
    float2 xy = float2(vso.posSS.x, -vso.posSS.y);
    float2 a = (xy / vso.posSS.w) * 0.5 + 0.5;
    xy = float2(vso.prevPos.x, -vso.prevPos.y);
    float2 b = (xy / vso.prevPos.w) * 0.5 + 0.5;
    
    pso.velocity = float4(a-b, 0.0f, 1.0f);
    return pso;
}