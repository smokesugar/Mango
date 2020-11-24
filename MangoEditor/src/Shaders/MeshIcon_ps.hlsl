
float4 main(float3 norm : Normal) : SV_Target
{
    float3 N = normalize(norm);
    float3 L = normalize(float3(1.0f, 2.0f, -0.5f));
    
    float dif = max(dot(N, L), 0.0f);
    
    return float4(dif.xxx, 1.0f);
}