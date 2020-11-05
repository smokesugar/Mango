

float main(float depth : Depth, bool frontFacing : SV_IsFrontFace) : SV_Depth
{
    float d = depth;
    d -= frontFacing ? 0.01f : 0.0f;
    return d;
}