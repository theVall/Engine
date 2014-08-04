
//  Pixel shader for terrain coloring and lighting

Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
    float padding;
};

struct PixelInputType
{
float4 position : SV_POSITION;
float2 tex : TEXCOORD0;
float3 normal : NORMAL;
float4 color : COLOR;
};

// Entry point main method
float4 Main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color = ambientColor;

    textureColor = shaderTexture.Sample(SampleType, input.tex);

    lightDir = -lightDirection;
    lightIntensity = saturate(dot(input.normal, lightDir));

    if (lightIntensity > 0.0f)
    {
        color += (diffuseColor*lightIntensity);
    }

    color = color * textureColor;

    // combine with color map value
    color = saturate(color);

    //lerp(color, input.color, float4(0.5f, 0.5f, 0.5f, 0.5f));

    color.w = 1.0;
    // DEBUG ONLY
    //color = float4(1,1,1,1);

    return color;
}
