
// constant buffer
cbuffer GradientBuffer
{
    float4 apexColor;
    float4 centerColor;
};

// Typedefs
struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 domePosition : TEXCOORD0;
};

float4 Main(PixelInputType input) : SV_TARGET
{
    float height;
    float4 outputColor;
    
    // Determine the position on the sky dome where this pixel is located.
    height = input.domePosition.y;

    height = clamp(height, 0.0f, 1.0f);

    // Determine the gradient color by interpolating between the apex and center
    // based on the height of the pixel in the sky dome.
    outputColor = lerp(centerColor, apexColor, height);

    return outputColor;
}