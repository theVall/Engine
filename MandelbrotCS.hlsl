
// number of threads are dispatch size per dimension times the block size.
// e.g. dispatch(32, 32, 1) -> (32*16, 32*16, 1*1) -> 512 threads
#define BLOCK_SIZE_X 16
#define BLOCK_SIZE_Y 16

cbuffer ImmutableConstBuf : register(b0)
{
    uint heightMapDim;
};

cbuffer PerFrameConstBuf : register(b1)
{
    float upperLeftX;
    float upperLeftY;
    float lowerRightX;
    float lowerRightY;
    float maxIterations;
    // size should be odd to have an unambiguous center
    float maskSize;
    float4 mask[57];
};

RWStructuredBuffer<float> outHeightBuf : register(u0);
RWStructuredBuffer<float> outFilteredBuf : register(u1);

[numthreads(BLOCK_SIZE_X, BLOCK_SIZE_Y, 1)]
void Main( uint3 DTid : SV_DispatchThreadID )
{
    int outputId = DTid.x * heightMapDim + DTid.y;

    float xStride = (lowerRightX - upperLeftX) / heightMapDim;
    float yStride = (upperLeftY - lowerRightY) / heightMapDim;

    float2 c = float2(upperLeftX + (float)DTid.x*xStride,
                      lowerRightY + (float)DTid.y*yStride);
    float2 z = c;

    outHeightBuf[outputId] = 0.0f;

    for (float i = 0.0f; i < maxIterations; i += 1.0f)
    {
        z = float2(z.x*z.x - z.y*z.y, 2.0f*z.x*z.y) + c;
        if (dot(z, z) > 4.0f)
        {
            outHeightBuf[outputId] = i;// / maxIterations;
            break;
        }
    }
}

[numthreads(BLOCK_SIZE_X, BLOCK_SIZE_Y, 1)]
void GaussBlur(uint3 DTid : SV_DispatchThreadID)
{
    uint N = heightMapDim*heightMapDim;
    uint index;
    uint maskId;
    uint maskSz = (uint)maskSize;

    float hh = 0.0f;
    // use integer division to get the down rounded value
    uint halfSize = maskSz / 2;
    bool border = false;

    //check if border
    if (!((DTid.x % heightMapDim < maskSz) ||
            ((DTid.y % heightMapDim) < maskSz) ||
            ((DTid.x % heightMapDim) > (heightMapDim - maskSz)) ||
            ((DTid.y % heightMapDim) >(heightMapDim - maskSz))))
    {
        for (uint i = 0; i < maskSz; ++i)
        {
            int row = i - halfSize;

            for (uint j = 0; j < maskSz; ++j)
            {
                int col = j - halfSize;

                index = (DTid.y + row) * heightMapDim + DTid.x + col;

                index = index >= N ? index % N : index;

                maskId = i*maskSz + j;

                hh += outHeightBuf[index] * ((float4)(mask[maskId / 4]))[maskId % 4];
            }
        }
        outFilteredBuf[DTid.y * heightMapDim + DTid.x] = hh;
    }
}