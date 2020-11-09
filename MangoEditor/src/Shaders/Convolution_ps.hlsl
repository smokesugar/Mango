
TextureCube cubemap : register(t0);
SamplerState sampler0 : register(s0);

const static float PI = 3.14159265359;

float4 main(float3 pos : Position) : SV_Target
{
    float3 N = normalize(pos);

    float3 up = float3(0, 1, 0);
    float3 right = normalize(cross(up, N));
    up = cross(N, right);

    float3 sampledColour = float3(0, 0, 0);
    float index = 0;
    for (float phi = 0; phi < 6.283; phi += 0.025)
    {
        for (float theta = 0; theta < 1.57; theta += 0.025)
        {
            float3 temp = cos(phi) * right + sin(phi) * up;
            float3 sampleVector = cos(theta) * N + sin(theta) * temp;
            sampledColour += cubemap.Sample(sampler0, sampleVector).rgb *
                                      cos(theta) * sin(theta);
            index++;
        }
    }

    return float4((PI * sampledColour / index), 1.0f);
}