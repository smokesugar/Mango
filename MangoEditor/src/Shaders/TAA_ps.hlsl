
Texture2D thisFrame : register(t0);
Texture2D lastFrame : register(t1);
Texture2D velocityBuffer : register(t2);

SamplerState sampler0 : register(s0);

struct VSOut
{
    float3 pos : Position;
    float2 uv : TexCoord;
    float4 svpos : SV_Position;
};

float4 main(VSOut vso) : SV_Target{
	float3 neighbourhood[9];

	uint width, height;
	thisFrame.GetDimensions(width, height);
	float2 pixelSize = 1.0f / float2(width, height);
	
    float2 vel = velocityBuffer.Sample(sampler0, vso.uv).xy;
	
	neighbourhood[0] = thisFrame.Sample(sampler0, vso.uv + float2(-1, -1) * pixelSize).xyz;
	neighbourhood[1] = thisFrame.Sample(sampler0, vso.uv + float2(+0, -1) * pixelSize).xyz;
	neighbourhood[2] = thisFrame.Sample(sampler0, vso.uv + float2(+1, -1) * pixelSize).xyz;
	neighbourhood[3] = thisFrame.Sample(sampler0, vso.uv + float2(-1, +0) * pixelSize).xyz;
	neighbourhood[4] = thisFrame.Sample(sampler0, vso.uv + float2(+0, +0) * pixelSize).xyz;
	neighbourhood[5] = thisFrame.Sample(sampler0, vso.uv + float2(+1, +0) * pixelSize).xyz;
	neighbourhood[6] = thisFrame.Sample(sampler0, vso.uv + float2(-1, +1) * pixelSize).xyz;
	neighbourhood[7] = thisFrame.Sample(sampler0, vso.uv + float2(+0, +1) * pixelSize).xyz;
	neighbourhood[8] = thisFrame.Sample(sampler0, vso.uv + float2(+1, +1) * pixelSize).xyz;

	float3 nmin = neighbourhood[0];
	float3 nmax = neighbourhood[0];
	for (int i = 1; i < 9; ++i) {
		nmin = min(nmin, neighbourhood[i]);
		nmax = max(nmax, neighbourhood[i]);
	}
    float2 histUv = vso.uv - vel;
	
    float3 histSample = lastFrame.Sample(sampler0, histUv).xyz;
    histSample = clamp(histSample, nmin, nmax);

    float subpixelCorrection = frac(max(abs(vel.x) * width, abs(vel.y) * height)) * 0.5f;
    float blend = saturate(lerp(0.05f, 0.8f, subpixelCorrection));
	
    if(histUv.x < 0.0f || histUv.x > 1.0f || histUv.y < 0.0f || histUv.y > 1.0f)
        blend = 1.0f;

    float3 curSample = neighbourhood[4];
	float3 c = lerp(histSample, curSample, blend.xxx);
    return float4(c, 1.0f);
}