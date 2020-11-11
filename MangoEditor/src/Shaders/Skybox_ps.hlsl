#include "ACES.hlsl"

TextureCube skybox : register(t0);
SamplerState sampler0 : register(s0);

float4 main(float3 pos : Position, float envStrength : EnvStrength) : SV_Target
{
    float3 fragColor = skybox.SampleLevel(sampler0, normalize(pos), 0).rgb;
    fragColor *= envStrength;
    fragColor = ACESFitted(fragColor);
    
    float gamma = 2.2f;
    fragColor = pow(fragColor, (1.0f / gamma).xxx);
    
    return float4(fragColor, 1.0);
}