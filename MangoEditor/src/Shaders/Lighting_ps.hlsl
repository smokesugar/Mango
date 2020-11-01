
Texture2D color : register(t0);
Texture2D normal : register(t1);
Texture2D depthBuffer : register(t2);

SamplerState sampler0 : register(s0);

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

float4 main (VSOut vso) : SV_Target
{
    float3 norm = normal.Sample(sampler0, vso.uv).xyz;
    if (dot(norm, norm) == 0.0f)
        discard;
    
    float4 colorSample = color.Sample(sampler0, vso.uv);
    float3 col = colorSample.rgb;
    float roughness = colorSample.a;
    
    float3 N = normalize(norm);
  
    float nonLinearDepth = depthBuffer.Sample(sampler0, vso.uv).r;
    float z = LinearizeDepth(nonLinearDepth);
    float3 P = GetPosition(vso.uv, z);
    
    float3 eye = mul(invView, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    float3 V = normalize(eye - P);
    
    const float3 lightPos = float3(1.0f, 3.0f, 1.0f);
    float3 L = normalize(lightPos - P);
    float3 H = normalize(L+V);
    
    float dif = max(dot(L, N), 0.0f);
    float spec = pow(max(dot(N, H), 0.0), 32.0f);
    
    float3 result = dif * col + spec * roughness;
    return float4(result, 1.0);
}