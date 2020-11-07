#include "ACES.hlsl"

struct VSOut
{
    float3 pos : Position;
    float2 uv : TexCoord;
    float4 svpos : SV_Position;
};

TextureCube skybox : register(t0);
SamplerState sampler0 : register(s0);

float4 main(VSOut vso) : SV_Target
{
    float3 fragColor = skybox.Sample(sampler0, normalize(vso.pos)).rgb;
    fragColor = ACESFitted(fragColor);
    
    float gamma = 2.2f;
    fragColor = pow(fragColor, (1.0f / gamma).xxx);
    
    return float4(fragColor, 1.0);
}