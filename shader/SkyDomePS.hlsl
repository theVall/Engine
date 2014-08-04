
Texture2D texSkyDome;

SamplerState samplerType;

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
float2 tex : TEXCOORD1;
};

float4 Main(PixelInputType input) : SV_TARGET
{
    float height;
    float4 outputColor;

    // Determine the position on the sky dome where this pixel is located.
    height = input.domePosition.y;

    height = clamp(height, 0.0f, 1.0f);

    float2 tex = input.domePosition.xy;
    //tex.x += 1.0;
    //tex.y += 1.0;
    tex.x /= 12.0f;
    tex.y /= 6.0f;

    tex.x += 0.55;
    tex.y += 0.55;


    // Determine the gradient color by interpolating between the apex and center
    // based on the height of the pixel in the sky dome.
    outputColor = lerp(centerColor, apexColor, height);

    //outputColor = texSkyDome.Sample(samplerType, tex);

    outputColor.a = 1.0f;

    return outputColor;
}