
// number of threads are dispatch size per dimension times the block size.
// e.g. dispatch(32, 32, 1) -> (32*16, 32*16, 1*1)
// -> 512 threads for each of the 2 dimensions (262144 threads in total)
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
    // Size should be odd to have an unambiguous center. Maximum size is 15 (see below).
    float maskSize;
    // maximum mask size is 4*512=2048 i.e. 45*45=2025 is the biggest mask with this setup
    // Note: maximum constant buffer size in DX11 is 4096
    float4 mask[512];
};

// output buffers
// height values
RWStructuredBuffer<float> outHeightBuf : register(u0);
// gauss filtered high values
RWStructuredBuffer<float> outFilteredBuf : register(u1);
// same as outFilteredBuf, but with texture view for PS texture sampling (minimap)
RWTexture2D<float> texHeight : register(u2);

// Approximate Mandelbrot set with the 'Pixel Game'.
[numthreads(BLOCK_SIZE_X, BLOCK_SIZE_Y, 1)]
void Main( uint3 DTid : SV_DispatchThreadID )
{
    int outputId = DTid.x * heightMapDim + DTid.y;

    float xStride = (lowerRightX - upperLeftX) / heightMapDim;
    float yStride = (upperLeftY - lowerRightY) / heightMapDim;

    float2 c = float2(upperLeftX + (float)DTid.x*xStride,
                      lowerRightY + (float)DTid.y*yStride);
    float2 z = c;

    // Height for pixel inside the Mandelbrot set are set to zero.
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


// Standard image post-processing filter.
// The Gaussian blur kernel/mask has to be calculated in advance by the host.
[numthreads(BLOCK_SIZE_X, BLOCK_SIZE_Y, 1)]
void GaussBlur(uint3 DTid : SV_DispatchThreadID)
{
    uint N = heightMapDim*heightMapDim;
    uint index;
    uint maskId;
    uint maskSz = (uint)maskSize;

    float result = 0.0f;
    // use integer division to get the rounded, lower value
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
                // get the correct value out of the 5-dimensional mask vector
                result += outHeightBuf[index] * ((float4)(mask[maskId / 4]))[maskId % 4];
            }
        }
        outFilteredBuf[DTid.y * heightMapDim + DTid.x] = result;
        texHeight[DTid.xy] = result / maxIterations;
    }
}