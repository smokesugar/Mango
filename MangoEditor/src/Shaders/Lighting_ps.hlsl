#include "ACES.hlsl"
#include "PBRFunctions.hlsl"

Texture2D color : register(t0);
Texture2D normal : register(t1);
Texture2D depthBuffer : register(t2);

Texture2DArray<float> directionalShadowmap0 : register(t3);
Texture2DArray<float> directionalShadowmap1 : register(t4);
Texture2DArray<float> directionalShadowmap2 : register(t5);
Texture2DArray<float> directionalShadowmap3 : register(t6);
Texture2DArray<float> directionalShadowmap4 : register(t7);

TextureCube irradianceMap : register(t8);
TextureCube prefilteredMap : register(t9);
Texture2D brdfLUT : register(t10);

SamplerState sampler0 : register(s0);
SamplerState sampler1 : register(s1);
SamplerState linearSampleClamp : register(s2);
SamplerComparisonState shadowSampler : register(s3);

struct VSOut
{
    float2 uv : TexCoord;
    float4 svpos : SV_Position;
};

#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 16
#define NUM_SHADOW_CASCADES 4

struct Light
{
    float3 Vector;
    float padding0;
    float3 Color;
    float padding1;
};

cbuffer LightingData : register(b0)
{
    matrix invView;
    Light pointLights[MAX_POINT_LIGHTS];
    Light directionalLights[MAX_DIRECTIONAL_LIGHTS];
    matrix directionalMatrices[MAX_DIRECTIONAL_LIGHTS * NUM_SHADOW_CASCADES];
    int numDirectionalLights;
    int numPointLights;
    float environmentStrength;
    float padding;
    float4 perspectiveValues;
    float4 cascadeEnds;
};

// Functions ------------------------------------------------------------------------------------

float2 GetSizeOfDirectionalShadowmap(int index)
{
    float2 size;
    float levels;
    switch (index)
    {
        case 0:
            directionalShadowmap0.GetDimensions(size.x, size.y, levels);
            break;
        case 1:
            directionalShadowmap1.GetDimensions(size.x, size.y, levels);
            break;
        case 2:
            directionalShadowmap2.GetDimensions(size.x, size.y, levels);
            break;
        case 3:
            directionalShadowmap3.GetDimensions(size.x, size.y, levels);
            break;
        case 4:
            directionalShadowmap4.GetDimensions(size.x, size.y, levels);
            break;
        default:
            directionalShadowmap0.GetDimensions(size.x, size.y, levels);
            break;
    }
    return size;
}

float SampleDirectionalShadowmap(int index, float3 uv, float comp)
{
    switch (index)
    {
        case 0:
            return directionalShadowmap0.SampleCmpLevelZero(shadowSampler, uv, comp);
        case 1:
            return directionalShadowmap1.SampleCmpLevelZero(shadowSampler, uv, comp);
        case 2:
            return directionalShadowmap2.SampleCmpLevelZero(shadowSampler, uv, comp);
        case 3:
            return directionalShadowmap3.SampleCmpLevelZero(shadowSampler, uv, comp);
        case 4:
            return directionalShadowmap4.SampleCmpLevelZero(shadowSampler, uv, comp);
        default:
            return directionalShadowmap0.SampleCmpLevelZero(shadowSampler, uv, comp);
    }
}

// Position ------------------------------------

float LinearizeDepth(float depth)
{
    float linearDepth = perspectiveValues.z / (depth + perspectiveValues.w);
    return linearDepth;
}

float3 GetPosition(float2 uv, float linearDepth)
{
    float2 clipSpace = uv * 2.0f - 1.0f;
    clipSpace.y *= -1.0f;
    
    float4 pos;
    pos.xy = clipSpace * perspectiveValues.xy * linearDepth;
    pos.z = linearDepth;
    pos.w = 1.0f;
    
    return mul(invView, pos).xyz;
}

// ---------------------------------------------

float3 CalculateLighting(float3 N, float3 V, float3 L, float3 H, float attenuation, float3 lightColor, float3 albedo, float roughness, float metallic)
{
    float3 radiance = lightColor * attenuation;
    
    float3 F0 = 0.04.xxx;
    F0 = lerp(F0, albedo, metallic);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);
    
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f);
    float3 specular = numerator / max(denominator, 0.001); // Avoid divide by zero
    
    float3 kS = F;
    float3 kD = 1.0f.xxx - kS;
    kD *= 1.0f - metallic;
    
    float NdotL = max(dot(N, L), 0.0f);
    float3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;
    return Lo;
}

// ---------------------------------------------------

float CalculateShadow(float depth, float3 worldPos, int lightIndex)
{
    int cascadeIndex = 0;
    
    for (int i = 0; i < NUM_SHADOW_CASCADES; i++)
    {
        if (depth < cascadeEnds[i])
        {
            cascadeIndex = i;
            break; 
        } else
            continue;
    }
    
    float4 position = mul(directionalMatrices[lightIndex*NUM_SHADOW_CASCADES+cascadeIndex], float4(worldPos, 1.0f));
    position /= position.w;
    
    if (position.z < 0.0f)
    {
        return 1.0f;
    }
    else
    {
        float2 uv = position.xy * 0.5 + 0.5;
        uv.y = 1.0f - uv.y;
    
        float shadowValue = 0.0f;
        float2 texelSize = 1.0f / GetSizeOfDirectionalShadowmap(lightIndex);
    
        for (int x = -2; x <= 2; x++)
        {
            for (int y = -2; y <= 2; y++)
            {
                shadowValue += SampleDirectionalShadowmap(lightIndex, float3(uv + float2(x, y) * texelSize, cascadeIndex), position.z).r;
            }
        }
    
        return shadowValue / 25.0f;
    }
}

// Main ------------------------------------------------------------------------------------------

float4 main (VSOut vso): SV_Target
{
    float4 normalSample = normal.Sample(sampler0, vso.uv);
    float3 norm = normalSample.xyz;
    if (dot(norm, norm) == 0.0f)
        discard;
    
    float metallic = normalSample.w;
    float4 colorSample = color.Sample(sampler0, vso.uv);
    float3 albedo = colorSample.rgb;
    float roughness = clamp(colorSample.a, 0.01f, 1.0f);
  
    float nonLinearDepth = depthBuffer.Sample(sampler0, vso.uv).r;
    float z = LinearizeDepth(nonLinearDepth);
    float3 worldPos = GetPosition(vso.uv, z);
    float3 eye = mul(invView, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;

    float3 N = normalize(norm);
    float3 V = normalize(eye - worldPos);
    float3 R = reflect(-V, N);
    
    // IBL --------------------------------------------------------
    
    float3 F0 = 0.04.xxx;
    F0 = lerp(F0, albedo, metallic);
    float NdotV = max(dot(N, V), 0.0f);
    float3 F = fresnelSchlickRoughness(NdotV, F0, roughness);
    float3 kS = F;
    float3 kD = 1.0f - kS;
    kD *= 1.0f - metallic;
    
    float3 irradiance = irradianceMap.SampleLevel(sampler1, N, 0).rgb * environmentStrength;
    float3 diffuse = irradiance * albedo;
    
    float w, h, mips;
    prefilteredMap.GetDimensions(0, w, h, mips);
    mips -= 1.0f;
    float3 prefilteredColor = prefilteredMap.SampleLevel(sampler1, R, roughness * mips).rgb * environmentStrength;
    float2 envBRDF = brdfLUT.Sample(linearSampleClamp, float2(NdotV, roughness)).rg;
    float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    
    float3 ambient = kD * diffuse + specular;
    
    // Lighting -------------------------------------------------------
    
    float3 Lo = 0.0f.xxx;
    
    {
        for (int i = 0; i < numPointLights; i++)
        {
            float3 L = pointLights[i].Vector - worldPos;
        
            float distance = length(L);
            float attenuation = 1.0f / (distance * distance);
            L /= distance;
        
            float3 H = normalize(V + L);
        
            Lo += CalculateLighting(N, V, L, H, attenuation, pointLights[i].Color, albedo, roughness, metallic);
        }
    }
    
    {
        for (int i = 0; i < numDirectionalLights; i++)
        {
            float3 L = normalize(directionalLights[i].Vector);
            float3 H = normalize(V + L);
            float attenuation = 1.0f;
            
            float shadowValue = CalculateShadow(z, worldPos, i);
            
            Lo += shadowValue * CalculateLighting(N, V, L, H, attenuation, directionalLights[i].Color, albedo, roughness, metallic);
        }
    }
    
    // ----------------------------------------------------------------
    
    float3 fragColor = Lo + ambient;
    fragColor = ACESFitted(fragColor);
    
    float gamma = 2.2f;
    fragColor = pow(fragColor, (1.0f / gamma).xxx);
    
    return float4(fragColor, 1.0);
}