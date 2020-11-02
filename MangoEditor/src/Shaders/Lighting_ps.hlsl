
Texture2D color : register(t0);
Texture2D normal : register(t1);
Texture2D depthBuffer : register(t2);

SamplerState sampler0 : register(s0);

#define PI 3.14159265359

struct VSOut
{
    float2 uv : TexCoord;
    float4 svpos : SV_Position;
};

cbuffer LightingData : register(b0)
{
    matrix invView;
    float4 perspectiveValues;
};

// Functions ------------------------------------------------------------------------------------

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

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

// Main ------------------------------------------------------------------------------------------

float4 main(VSOut vso) : SV_Target
{
    float4 normalSample = normal.Sample(sampler0, vso.uv);
    float3 norm = normalSample.xyz;
    if (dot(norm, norm) == 0.0f)
        discard;
    
    float metallic = normalSample.w;
    
    float4 colorSample = color.Sample(sampler0, vso.uv);
    float3 albedo = colorSample.rgb;
    float roughness = colorSample.a;
  
    float nonLinearDepth = depthBuffer.Sample(sampler0, vso.uv).r;
    float z = LinearizeDepth(nonLinearDepth);
    float3 worldPos = GetPosition(vso.uv, z);
    float3 eye = mul(invView, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    float3 lightColor = 1.0f.xxx;
    
    // Lighting -------------------------------------------------------
    
    float3 N = normalize(norm);
    float3 V = normalize(eye - worldPos);
    
    float3 L = normalize(float3(2.0f, 3.0f, 0.0f));
    float3 H = normalize(V + L);
    
    float attenuation = 1.0f;
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
    
    // ----------------------------------------------------------------
    
    float3 fragColor = Lo;
    
    float gamma = 2.2f;
    fragColor = pow(fragColor, (1.0f/gamma).xxx);
    
    return float4(fragColor, 1.0);
}