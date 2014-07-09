
//  Compute shader for ocean simulation.

#define PI 3.1415926536
#define BLOCK_SIZE_X 16
#define BLOCK_SIZE_Y 16

cbuffer ImmutableConstBuf : register(b0)
{
    uint actualDim;
    uint inWidth;
    uint outWidth;
    uint outHeight;
};

cbuffer PerFrameConstBuf : register(b1)
{
    float time;
    float choppyScale;
    // float gridLength;
};

StructuredBuffer<float2>	inputH0Buf		: register(t0);
StructuredBuffer<float>		inputOmegaBuf	: register(t1);
RWStructuredBuffer<float2>	outputHtBuf		: register(u0);
RWStructuredBuffer<float2>	outputDxBuf		: register(u1);
RWStructuredBuffer<float2>	outputDyBuf		: register(u2);

[numthreads(BLOCK_SIZE_X, BLOCK_SIZE_Y, 1)]
void Main(uint3 threadId : SV_DispatchThreadID)
{
    int inputId  = threadId.y * inWidth + threadId.x;
    int inputMId = (actualDim - threadId.y) * inWidth + (actualDim - threadId.x);
    int outputId = threadId.y * outWidth + threadId.x;

    // H(0) -> H(t)
    float2 waveVec = inputH0Buf[inputId];
    float2 waveVecM = inputH0Buf[inputMId];
    float sinOmega;
    float cosOmega;
    sincos(inputOmegaBuf[inputId] * time, sinOmega, cosOmega);

    float2 ht;
    ht.x = (waveVec.x + waveVecM.x) * cosOmega - (waveVec.y + waveVecM.y) * sinOmega;
    ht.y = (waveVec.x - waveVecM.x) * sinOmega + (waveVec.y - waveVecM.y) * cosOmega;

    // H(t) -> Dx(t), Dy(t)
    float kx = threadId.x - actualDim * 0.5f;
    float ky = threadId.y - actualDim * 0.5f;
    float sqrK = kx * kx + ky * ky;
    float rsqrK = 0;
    // avoid zero div
    if (sqrK > 1e-12f)
    {
        rsqrK = 1 / sqrt(sqrK);
    }

    kx *= rsqrK;
    ky *= rsqrK;
    float2 dtx = float2(ht.y * kx, -ht.x * kx);
    float2 dty = float2(ht.y * ky, -ht.x * ky);

    if ((threadId.x < outWidth) && (threadId.y < outHeight))
    {
        outputHtBuf[outputId] = ht;
        outputDxBuf[outputId] = dtx;
        outputDyBuf[outputId] = dty;
    }
}