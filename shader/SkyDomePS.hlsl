
Texture2D texSkyDome;

SamplerState samplerType;

// constant buffer
cbuffer GradientBuffer
{
    float4 apexColor;
    float4 centerColor;
    float3 lightDir;
    float3 eyeVec;
};

// Typedefs
struct PixelInputType
{
float4 position : SV_POSITION;
float4 domePosition : TEXCOORD0;
float2 tex : TEXCOORD1;
float4 positionModel : POSITIONMODEL;
};


// primitive simulation of non-uniform atmospheric fog
float3 CalcFogColor(float3 pixelToLightVec, float3 pixelToEyeVec)
{
    return lerp(float3(0.6, 0.6, 0.7),
                float3(1.0, 1.1, 1.4),
                0.5*dot(pixelToLightVec, -pixelToEyeVec) + 0.5);
}


float4 Main(PixelInputType input) : SV_TARGET
{
    float height;
    float4 color;

    // Determine the position on the sky dome where this pixel is located.
    height = input.domePosition.y;

    height = clamp(height, 0.0f, 1.0f);

    // gradient coloring
    //color = lerp(centerColor, apexColor, height);

    // texture coloring
    color = texSkyDome.Sample(samplerType, input.tex);

    // calculate fog
    //float3 pixelPos = input.positionModel.xyz;
    //float3 pixelToLightVec = normalize(lightDir - pixelPos);
    //float3 pixelToEyeVec = normalize(eyeVec - pixelPos);

    //float3 fogColor = CalcFogColor(pixelToLightVec, pixelToEyeVec);
    //color.rgb = lerp(color.rgb, fogColor, pow(saturate(input.tex.y), 10.0f));

    color.a = 1.0f;

    return color;
}