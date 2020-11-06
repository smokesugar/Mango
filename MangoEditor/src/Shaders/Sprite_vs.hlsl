
struct VSOut {
    float4 posSS : ScreenSpacePosition;
    float4 prevPos : PreviousPosition;
	float2 uv : TexCoord;
	float4 col : Color;
	float4 svpos : SV_Position;
};

cbuffer Uniforms : register(b0) {
    matrix prevMVp;
	matrix MVP;
	float4 color;
}

VSOut main(float3 pos : Position, float2 uv : TexCoord) {
	VSOut vso;
	
	vso.uv = uv;
    vso.svpos = mul(MVP, float4(pos, 1.0f));
    vso.posSS = vso.svpos;
	vso.col = color;
	vso.prevPos = mul(prevMVp, float4(pos, 1.0f));
	
	return vso;
}