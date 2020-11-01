
struct VSOut {
	float3 pos : Position;
    float4 posSS : ScreenSpacePosition;
    float4 prevPos : PreviousPosition;
	float2 uv : TexCoord;
	float4 col : Color;
	float4 svpos : SV_Position;
};

cbuffer GlobalUniforms : register(b0) {
    matrix prevViewProjection;
	matrix viewProjection;
};

cbuffer IndividualUniforms : register(b1) {
    matrix prevModel;
	matrix model;
	float4 color;
}

VSOut main(float3 pos : Position, float2 uv : TexCoord) {
	VSOut vso;
	
	float4 worldPos = mul(model, float4(pos, 1.0));
	vso.pos = worldPos.xyz;
	vso.uv = uv;
	vso.svpos = mul(viewProjection, worldPos);
   
	vso.col = color;
	
    vso.posSS = vso.svpos;
	vso.prevPos = mul(prevViewProjection, mul(prevModel, float4(pos, 1.0f)));
	
	return vso;
}