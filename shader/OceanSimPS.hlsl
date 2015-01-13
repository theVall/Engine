
//  Pixel shader for generating a displacement map from the position buffers
//  and a gradient/folding map from the displacement.

Texture2D displacementMap : register(t0);
SamplerState linearSampler : register(s0);

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
};

struct PixelInputType
{
float4 position		: SV_POSITION;
float2 tex  		: TEXCOORD0;
};

// Data buffers for Dx, Dy and Dz
StructuredBuffer<float2> inputDxBuf	: register(t0);
StructuredBuffer<float2> inputDyBuf	: register(t1);
StructuredBuffer<float2> inputDzBuf : register(t2);

// Put Dx, Dy, Dz into a displacement texture
float4 UpdateDisplacementPS(PixelInputType input) : SV_Target
{
    uint xID = (uint)(input.tex.x * (float)outWidth);
    uint yID = (uint)(input.tex.y * (float)outHeight);
    uint addr = outWidth * yID + xID;

    // sign correction
    int sign = ((xID + yID) & 1) ? -1 : 1;

    float dx = inputDxBuf[addr].x * sign * choppyScale;
    float dy = inputDyBuf[addr].x * sign * choppyScale;
    float dz = inputDzBuf[addr].x * sign;

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

    float3 displaceLeft  = displacementMap.Sample(linearSampler, tcLeft ).xyz;
    float3 displaceRight = displacementMap.Sample(linearSampler, tcRight).xyz;
    float3 displaceBack  = displacementMap.Sample(linearSampler, tcBack ).xyz;
    float3 displaceFront = displacementMap.Sample(linearSampler, tcFront).xyz;

    float2 gradient = { -(displaceRight.z - displaceLeft.z), -(displaceFront.z - displaceBack.z) };

    // Calculate Jacobian for folding
    float2 Dx = (displaceRight.xy - displaceLeft.xy) * choppyScale * 0.25f;
    float2 Dy = (displaceFront.xy - displaceBack.xy) * choppyScale * 0.25f;
    float jacobian = (1.0f + Dx.x) * (1.0f + Dy.y) - Dx.y * Dy.x;

    float fold = max(1.0f - jacobian, 0.0f);

    // Output
    return float4(gradient, 0.0f, fold);
}
