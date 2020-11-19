
Texture2D<float> depthBuffer : register(t0);
Texture2D normalBuffer : register(t1);

SamplerState sampler0 : register(s0);

struct VSOut
{
    float2 uv : TexCoord;
    float4 svpos : SV_Position;
};

cbuffer constantBuffer : register(b0)
{
    matrix invView;
    matrix view;
    matrix projection;
    float4 perspectiveValues;
    float randomSeed;
    float3 padding;
};

float LinearizeDepth(float depth)
{
    float linearDepth = perspectiveValues.z / (depth + perspectiveValues.w);
    return linearDepth;
}

float3 GetViewPosition(float2 uv, float linearDepth)
{
    float2 clipSpace = uv * 2.0f - 1.0f;
    clipSpace.y *= -1.0f;
    
    float3 pos;
    pos.xy = clipSpace * perspectiveValues.xy * linearDepth;
    pos.z = linearDepth;
    
    return pos;
}

float nrand(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

float3 GetRandomInHemisphere(float scale, float2 uv)
{
    float3 pos = float3(
        nrand(uv * 2) * 2.0f - 1.0f,
        nrand(uv * 3) * 2.0f - 1.0f,
        nrand(uv * 4)
    );
    pos = normalize(pos);
    float shift = lerp(0.1f, 1.0f, scale * scale);
    pos *= shift;
    return pos;
};

float3 GetRandomOrientation(float2 uv)
{
    float3 noise = float3(
        nrand(uv * 5) * 2.0f - 1.0f,
        nrand(uv * 6) * 2.0f - 1.0f,
        0.0f
    );
    return noise;
};


float4 main(VSOut vso) : SV_Target
{
    float nonLinearDepth = depthBuffer.Sample(sampler0, vso.uv).r;
    float z = LinearizeDepth(nonLinearDepth);
    float3 fragPos = GetViewPosition(vso.uv, z);
    float3 normal = normalize(mul((float3x3) view, normalize(normalBuffer.Sample(sampler0, vso.uv).xyz)));
    
    float randSeed = nrand(vso.uv*randomSeed);
    
    uint width, height;
    depthBuffer.GetDimensions(width, height);
    
    float3 randomVec = GetRandomOrientation(vso.uv);
    float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normal);
    
    const int kernelSize = 64;
    const float radius = 1.0f;
    const float bias = 0.025;
    
    float occlusion = 0.0f;
    for (int i = 0; i < kernelSize; i++)
    {
        float3 samplePos = mul(GetRandomInHemisphere(float(i) / float(kernelSize), vso.uv * i * randSeed), TBN);
        samplePos = fragPos + samplePos * radius;
        
        float4 offset = float4(samplePos, 1.0f);
        offset = mul(projection, offset);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5f + 0.5f;
        offset.y = 1.0f - offset.y;
        
        float sampleDepth = depthBuffer.Sample(sampler0, offset.xy);
        sampleDepth = LinearizeDepth(sampleDepth);
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth <= samplePos.z - bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0f - (occlusion/kernelSize);
    
    return occlusion.xxxx;
}