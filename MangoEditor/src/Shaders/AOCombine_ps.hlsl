
Texture2D aoTexture : register(t0);
Texture2D litImage : register(t1);

SamplerState sampler0 : register(s0);

struct VSOut
{
    float2 uv : TexCoord;
    float4 svpos : SV_Position;
};


float4 main(VSOut vso) : SV_Target
{
    uint width, height;
    aoTexture.GetDimensions(width, height);
    
    //float2 texelSize = 1.0f / float2(float(width), float(height));
    //float result = 0.0f;
    //for (int x = -2; x < 2; ++x)
    //{
    //    for (int y = -2; y < 2; ++y)
    //    {
    //        float2 offset = float2(float(x), float(y)) * texelSize;
    //        result += aoTexture.Sample(sampler0, vso.uv + offset).r;
    //    }
    //}
    //float occlusion = result/(16.0f);
    float occlusion = aoTexture.Sample(sampler0, vso.uv).r;
    
    return litImage.Sample(sampler0, vso.uv) * occlusion;
}