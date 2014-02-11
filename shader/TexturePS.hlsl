
//  Pixel shader for textured surfaces

Texture2D shaderTexture;
SamplerState sampleType;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};


//  Entry point main method.
float4 Main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;

    textureColor = shaderTexture.Sample(sampleType, input.tex);

    return textureColor;
}