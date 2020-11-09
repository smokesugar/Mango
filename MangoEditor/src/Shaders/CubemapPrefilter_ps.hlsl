#include "Hammersly.hlsl"
#include "PBRFunctions.hlsl"

TextureCube cubemap : register(t0);
SamplerState sampler0 : register(s0);

cbuffer RoughnessBuffer : register(b0)
{
    float4 rough;
}

float4 main(float3 pos : Position) : SV_Target
{
    float roughness = rough.r;
    
    float3 N = normalize(pos);
    float3 R = N;
    float3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0;
    float3 prefilteredColor = 0.0.xxx;
    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, N, roughness);
        float3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0)
        {
            float HdotV = max(dot(H, V), 0.0f);
            float NdotH = max(dot(N, H), 0.0f);
        
            float D = DistributionGGX(N, H, roughness);
            float pdf = (D * NdotH / (4.0 * HdotV)) + 0.0001;

            uint w, h;
            cubemap.GetDimensions(w, h);
            float resolution = w;
            float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
            
            prefilteredColor += cubemap.SampleLevel(sampler0, L, mipLevel).rgb * NdotL;
            totalWeight += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;

    return float4(prefilteredColor, 1.0f);
}