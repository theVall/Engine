
#define SQRT_1_2 0.707106781186
#define PI 3.141592653

#define MUL_RE(a, b) (a.y*b.y - a.x*b.x)
#define MUL_IM(a, b) (a.y*b.x + a.x*b.y)

#define mul_p0q1(a) (a)
#define mul_p0q2 mul_p0q1
#define mul_p0q4 mul_p0q2
#define mul_p2q4 mul_p1q2

// Constant buffer, called on per frame basis.
cbuffer PerFrameConstBuf : register(b0)
{
    uint threadCnt;
    uint outStride;
    uint inStride;
    uint phaseStride;
    float phase;
};


float2 mul_1(float2 a, float2 b)
{
    float2 x;
    x.y = MUL_RE(a, b);
    x.x  = MUL_IM(a, b);
    return x;
}

float2 mul_p1q2(float2 a)
{
    return float2(a.y, -a.x);
}

float2 mul_p1q4(float2 a)
{
    return (float2)(SQRT_1_2)*(float2)(a.x + a.y, -a.x + a.y);
}

float2 mul_p3q4(float2 a)
{
    return (float2)(SQRT_1_2)*(float2)(-a.x + a.y, -a.x - a.y);
}

float2 exp_alpha_1(float alpha)
{
    float cs;
    float sn;
    sincos(alpha, sn, cs);
    return float2(cs, sn);
}

// Input and output data buffers.
StructuredBuffer<float2>	inputBuf : register(t0);
RWStructuredBuffer<float2>	outputBuf : register(u0);

[numthreads(128, 1, 1)]
void Main(uint3 threadId : SV_DispatchThreadID)
{
    uint t = threadCnt;
    uint p = inStride;
    uint q = outStride;

    // id magic...
    uint k = threadId.x & (p - 1);
    uint inAddr = threadId.x;
    uint outAddr = ((threadId.x - k) << 3) + k;

    float alpha = -PI*(float)k / (float)(4 * p);

    float2 u[8];
    float2 v[8];

    // twiddle and calculations
    u[0] =                               inputBuf[inAddr + 0*t];
    u[1] = mul_1(exp_alpha_1(1 * alpha), inputBuf[inAddr + 1*t]);
    u[2] = mul_1(exp_alpha_1(2 * alpha), inputBuf[inAddr + 2*t]);
    u[3] = mul_1(exp_alpha_1(3 * alpha), inputBuf[inAddr + 3*t]);
    u[4] = mul_1(exp_alpha_1(4 * alpha), inputBuf[inAddr + 4*t]);
    u[5] = mul_1(exp_alpha_1(5 * alpha), inputBuf[inAddr + 5*t]);
    u[6] = mul_1(exp_alpha_1(6 * alpha), inputBuf[inAddr + 6*t]);
    u[7] = mul_1(exp_alpha_1(7 * alpha), inputBuf[inAddr + 7*t]);

    v[0] = u[0] + u[4];
    v[4] = mul_p0q4(u[0] - u[4]);
    v[1] = u[1] + u[5];
    v[5] = mul_p1q4(u[1] - u[5]);
    v[2] = u[2] + u[6];
    v[6] = mul_p2q4(u[2] - u[6]);
    v[3] = u[3] + u[7];
    v[7] = mul_p3q4(u[3] - u[7]);

    u[0] = v[0] + v[2];
    u[2] = mul_p0q2(v[0] - v[2]);
    u[1] = v[1] + v[3];
    u[3] = mul_p1q2(v[1] - v[3]);
    u[4] = v[4] + v[6];
    u[6] = mul_p0q2(v[4] - v[6]);
    u[5] = v[5] + v[7];
    u[7] = mul_p1q2(v[5] - v[7]);

    outputBuf[outAddr + 0*q] = u[0] + u[1];
    outputBuf[outAddr + 1*q] = u[4] + u[5];
    outputBuf[outAddr + 2*q] = u[2] + u[3];
    outputBuf[outAddr + 3*q] = u[6] + u[7];
    outputBuf[outAddr + 4*q] = u[0] - u[1];
    outputBuf[outAddr + 5*q] = u[4] - u[5];
    outputBuf[outAddr + 6*q] = u[2] - u[3];
    outputBuf[outAddr + 7*q] = u[6] - u[7];
}