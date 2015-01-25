
// dispatched with (32,32,1) this results in (32*16, 32*16, 1*1) -> (512, 512, 1) threads
#define BLOCK_SIZE_X 16
#define BLOCK_SIZE_Y 16

cbuffer ImmutableConstBuf : register(b0)
{
    // should be 512 with 16er block size to get the right amount of threads
    uint heightMapDim;
};

cbuffer PerFrameConstBuf : register(b1)
{
    float upperLeftX;
    float upperLeftY;
    float lowerRightX;
    float lowerRightY;
    float maxIterations;
};

RWStructuredBuffer<float> outputHeightBuf : register(u0);

[numthreads(BLOCK_SIZE_X, BLOCK_SIZE_Y, 1)]
void Main( uint3 DTid : SV_DispatchThreadID )
{
    int outputId = DTid.x * heightMapDim + DTid.y;

    float xStride = (lowerRightX - upperLeftX) / heightMapDim;
    float yStride = (upperLeftY - lowerRightY) / heightMapDim;

    float2 c = float2(upperLeftX + (float)DTid.x*xStride,
                      lowerRightY + (float)DTid.y*yStride);
    float2 z = c;

    outputHeightBuf[outputId] = 1.0f;

    for (float i = 0.0f; i < maxIterations; i += 1.0f)
    {
        z = float2(z.x*z.x - z.y*z.y, 2.0f*z.x*z.y) + c;
        if (dot(z, z) > 4.0f)
        {
            outputHeightBuf[outputId] = i / maxIterations;
            break;
        }
    }
}