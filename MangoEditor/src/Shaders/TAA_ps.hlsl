
Texture2D thisFrame : register(t0);
Texture2D lastFrame : register(t1);

SamplerState sampler0 : register(s0);

struct VSOut {
	float3 pos : Position;
	float2 uv : TexCoord;
	float4 svpos : SV_Position;
};

float4 main(VSOut vso) : SV_Target{
	float3 neighbourhood[9];

	uint width, height;
	thisFrame.GetDimensions(width, height);
	float2 pixelSize = 1.0f / float2(width, height);
	
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

	float3 histSample = clamp(lastFrame.Sample(sampler0, vso.uv).xyz, nmin, nmax);
	float3 curSample = neighbourhood[4];

	float blend = 0.5f;

	float3 c = lerp(histSample, curSample, float3(blend, blend, blend));
	return float4(c, 1.0f);
}