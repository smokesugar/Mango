
struct VSOut {
	float3 pos : Position;
	float4 svpos : SV_Position;
};

VSOut main(float3 pos : Position) {
	VSOut vso;
	vso.pos = pos;
	vso.svpos = float4(pos, 1.0);
	return vso;
}