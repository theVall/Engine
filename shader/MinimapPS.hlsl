
//  Pixel shader for the Minimap

Texture2D<float> texHeight : register(t0);
SamplerState sampleType : register (s1);

cbuffer PerFrameConstBufPS : register(b1)
{
    float mapWidth;
    float mapHeight;
    float xRes;
    float yRes;
};

struct PixelInputType
{
float4 position : SV_POSITION;
float2 tex : TEXCOORD0;
};


// Entry point main method.
float4 Main(PixelInputType input) : SV_TARGET
{
    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 output = float4(1.0f, 1.0f, 1.0f, 1.0f);

    float2 tex = input.tex.yx;

    // draw white border
    if (tex.y*mapWidth > mapWidth - 5 || tex.y*mapWidth < 5 ||
    tex.x*mapHeight > mapHeight - 5 || tex.x*mapHeight < 5)
    {
        return output;
    }

    float height = (texHeight.Sample(sampleType, tex).r);
    height /= 10.0f;

    if (height == 0.0f)
    {
        color.rgb = float3(0.9f, 0.9f, 0.0f);
    }
    else
    {
        if (height < 0.04f)
        {
            height = 3.0f * sqrt(height);
        }
        color.r = height / 0.9f;
        color.g = height / 0.9f;
        color.b = height / 0.3f;
    }

    output = color * 1.0f;
    output = saturate(output);

    return output;
}