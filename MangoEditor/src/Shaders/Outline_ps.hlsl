
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

float4 main(float2 uv : TexCoord) : SV_Target
{
    float3 samples[9];
    
    uint width, height;
    texture0.GetDimensions(width, height);
    float2 pixelSize = 1.0f / float2(width, height);
   
    samples[4] = texture0.Sample(sampler0, uv + float2(+0, +0) * pixelSize).rgb;
    
    if (any(samples[4] != float3(0.0f, 0.0f, 0.0f)))
        discard;
    
    samples[0] = texture0.Sample(sampler0, uv + float2(-1, -1) * pixelSize).rgb;
    samples[1] = texture0.Sample(sampler0, uv + float2(+0, -1) * pixelSize).rgb;
    samples[2] = texture0.Sample(sampler0, uv + float2(+1, -1) * pixelSize).rgb;
    samples[3] = texture0.Sample(sampler0, uv + float2(-1, +0) * pixelSize).rgb;
    samples[5] = texture0.Sample(sampler0, uv + float2(+1, +0) * pixelSize).rgb;
    samples[6] = texture0.Sample(sampler0, uv + float2(-1, +1) * pixelSize).rgb;
    samples[7] = texture0.Sample(sampler0, uv + float2(+0, +1) * pixelSize).rgb;
    samples[8] = texture0.Sample(sampler0, uv + float2(+1, +1) * pixelSize).rgb;
    
    float3 maxCol = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 9; i++)
    {
        maxCol = max(maxCol, samples[i]);
    }
    
    if (all(maxCol == float3(0.0f, 0.0f, 0.0f)))
        discard;
        
    return float4(maxCol, 1.0f);
}