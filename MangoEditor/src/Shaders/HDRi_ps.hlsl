
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

const static float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

float4 main(float3 pos : Position) : SV_Target
{
    return texture0.Sample(sampler0, SampleSphericalMap(normalize(pos)));
}