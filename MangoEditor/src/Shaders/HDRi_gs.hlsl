
cbuffer Transforms : register(b0)
{
    matrix mvp[6];
};

struct GSOut
{
    float3 pos : Position;
    float4 svpos : SV_Position;
    uint arrayIndex : SV_RenderTargetArrayIndex;
};

struct VSOut
{
    float4 svpos : SV_Position;
};

[maxvertexcount(18)]
void main(triangle VSOut input[3], inout TriangleStream<GSOut> outputStream)
{
    for (int j = 0; j < 6; j++)
    {
        for (int i = 0; i < 3; i++)
        {
            GSOut gso;
            gso.svpos = mul(mvp[j], input[i].svpos);
            gso.pos = input[i].svpos;
            gso.arrayIndex = j;
            outputStream.Append(gso);
        }
        outputStream.RestartStrip();

    }
}