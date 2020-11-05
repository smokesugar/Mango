#define NUM_SHADOW_CASCADES 4

cbuffer Uniforms : register(b0)
{
    matrix mvp[NUM_SHADOW_CASCADES];
}

struct VSOut
{
    float4 svpos : SV_Position;
};

struct GSOut
{
    float depth : Depth;
    float4 svpos : SV_Position;
    uint arrayIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(3 * NUM_SHADOW_CASCADES)]
void main(triangle VSOut input[3], inout TriangleStream<GSOut> outputStream)
{
    for (int j = 0; j < NUM_SHADOW_CASCADES; j++)
    {
        for (int i = 0; i < 3; i++)
        {
            GSOut gso;
            gso.svpos = mul(mvp[j], input[i].svpos);
            gso.depth = gso.svpos.z / gso.svpos.w;
            gso.arrayIndex = j;
            outputStream.Append(gso);
        }
        outputStream.RestartStrip();

    }
}