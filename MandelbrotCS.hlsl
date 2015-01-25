
#define BLOCK_SIZE_X 16
#define BLOCK_SIZE_Y 16

cbuffer ImmutableConstBuf : register(b0)
{
    uint heightMapDim;
};

cbuffer PerFrameConstBuf : register(b1)
{
    uint upperLeftX;
    uint upperLeftY;
    uint lowerRightX;
    uint lowerRightY;
    uint maxIterations;
};

RWStructuredBuffer<float> outputHeightBuf : register(u0);
StructuredBuffer<float> outputHeightBuf2 : register(t0);
//RWStructuredBuffer<float3> outputColorBuf	: register(u1);

[numthreads(BLOCK_SIZE_X, BLOCK_SIZE_X, 1)]
void Main( uint3 DTid : SV_DispatchThreadID )
{
    int outputId = DTid.y * heightMapDim + DTid.x;

    float xStride = (float)(lowerRightX - upperLeftX) / (float)heightMapDim;
    float yStride = (float)(upperLeftY - lowerRightY) / (float)heightMapDim;

    float2 c = float2((float)DTid.x * xStride, (float)DTid.y * yStride);
    c = normalize(c);

    float2 z = c;

    outputHeightBuf[outputId] = 1.0f;

    for (uint i = 0; i < maxIterations; ++i)
    {
        z = float2(z.x*z.x - z.y*z.y, 2.0f*z.x*z.y) + c;
        if (dot(z, z) > 4.0f)
        {
            outputHeightBuf[outputId] = (float)i / (float)maxIterations;
            break;
        }
    }
}