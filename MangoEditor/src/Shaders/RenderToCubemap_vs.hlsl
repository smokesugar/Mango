
 
float4 main(float3 pos : Position, float3 norm : Normal, float2 uv : TexCoord) : SV_Position
{
    return float4(pos, 1.0f);

}