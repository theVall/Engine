
//  Pixel shader for ocean simulation

Texture2D displacementMap : register(t0);
SamplerState LinearSampler : register(s0);

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

struct PixelInputType
{
float4 position		: SV_POSITION;
float2 tex  		: TEXCOORD0;
};

// Data buffers for Dx, Dy and Dz
StructuredBuffer<float2>	inputDxBuf		: register(t0);
StructuredBuffer<float2>	inputDyBuf		: register(t1);
StructuredBuffer<float2>	inputDzBuf		: register(t2);

// Post-FFT data wrap up: Dx, Dy, Dz -> Displacement
float4 UpdateDisplacementPS(PixelInputType input) : SV_Target
{
    uint xID = (uint)(input.tex.x * (float)outWidth);
    uint yID = (uint)(input.tex.y * (float)outHeight);
    uint addr = outWidth * yID + xID;

    int sign_correction = ((xID + yID) & 1) ? -1 : 1;

    float dx = inputDxBuf[addr].x * sign_correction * choppyScale;
    float dy = inputDyBuf[addr].x * sign_correction * choppyScale;
    float dz = inputDzBuf[addr].x * sign_correction;

    return float4(dx, dy, dz, 1);
}

// Displacement -> Normal, Folding
float4 NormalsFoldingsPS(PixelInputType input) : SV_Target
{
    // Sample neighbor texels
    float2 texelSize = float2(1.0f / (float)outWidth, 1.0f / (float)outHeight);

    float2 tcLeft  = float2(input.tex.x - texelSize.x, input.tex.y);
    float2 tcRight = float2(input.tex.x + texelSize.x, input.tex.y);
    float2 tcBack  = float2(input.tex.x, input.tex.y - texelSize.y);
    float2 tcFront = float2(input.tex.x, input.tex.y + texelSize.y);

    float3 displaceLeft  = displacementMap.Sample(LinearSampler, tcLeft ).xyz;
    float3 displaceRight = displacementMap.Sample(LinearSampler, tcRight).xyz;
    float3 displaceBack  = displacementMap.Sample(LinearSampler, tcBack ).xyz;
    float3 displaceFront = displacementMap.Sample(LinearSampler, tcFront).xyz;

    float2 gradient = { -(displaceRight.z - displaceLeft.z), -(displaceFront.z - displaceBack.z) };

    // Calculate Jacobian for folding
    float2 Dx = (displaceRight.xy - displaceLeft.xy) * choppyScale; // *gridLength;
    float2 Dy = (displaceFront.xy - displaceBack.xy) * choppyScale; // *gridLength;
    float jacobian = (1.0f + Dx.x) * (1.0f + Dy.y) - Dx.y * Dy.x;

    float fold = max(1.0f - jacobian, 0.0f);

    // Output
    return float4(gradient, 0.0f, fold);
}
