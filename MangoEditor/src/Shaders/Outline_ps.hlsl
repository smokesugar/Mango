
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer ColorBuffer : register(b0)
{
    float3 color;
    float padding;
};

float SampleTexture(float2 uv, float2 pixeloffset)
{
    float samples[9];
    
    uint width, height;
    texture0.GetDimensions(width, height);
    float2 pixelSize = 1.0f / float2(width, height);
    
    samples[0] = texture0.Sample(sampler0, uv + (float2(-1, -1)+pixeloffset) * pixelSize).r;
    samples[1] = texture0.Sample(sampler0, uv + (float2(+0, -1)+pixeloffset) * pixelSize).r;
    samples[2] = texture0.Sample(sampler0, uv + (float2(+1, -1)+pixeloffset) * pixelSize).r;
    samples[3] = texture0.Sample(sampler0, uv + (float2(-1, +0)+pixeloffset) * pixelSize).r;
    samples[4] = texture0.Sample(sampler0, uv + (float2(+0, +0)+pixeloffset) * pixelSize).r;
    samples[5] = texture0.Sample(sampler0, uv + (float2(+1, +0)+pixeloffset) * pixelSize).r;
    samples[6] = texture0.Sample(sampler0, uv + (float2(-1, +1)+pixeloffset) * pixelSize).r;
    samples[7] = texture0.Sample(sampler0, uv + (float2(+0, +1)+pixeloffset) * pixelSize).r;
    samples[8] = texture0.Sample(sampler0, uv + (float2(+1, +1)+pixeloffset) * pixelSize).r;
    
    float maxVal = 0.0f;
    for (int i = 0; i < 9; i++)
    {
        maxVal = max(maxVal, samples[i]);
    }
    
    return maxVal;
}

float4 main(float2 uv : TexCoord) : SV_Target
{
    float val = 0.0f;
    val += SampleTexture(uv, 0.0f.xx);
    
    float multiplier = 1.0f - texture0.Sample(sampler0, uv).r;
    val *= multiplier;
    return float4(color, val);
}