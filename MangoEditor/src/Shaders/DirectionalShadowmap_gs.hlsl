cbuffer Uniforms : register(b0)
{
    matrix prevModel;
    matrix mvp;
}

struct VSOut
{
    float4 svpos : SV_Position;
};

struct GSOut
{
    float depth : Depth;
    float4 svpos : SV_Position;
};

[maxvertexcount(3)]
void main(triangle VSOut input[3], inout TriangleStream<GSOut> outputStream)
{
    for (int i = 0; i < 3; i++)
    {
        GSOut gso;
        gso.svpos = mul(mvp, input[i].svpos);
        gso.depth = gso.svpos.z / gso.svpos.w;
        outputStream.Append(gso);
    }
}