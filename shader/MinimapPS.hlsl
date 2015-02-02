
//  Pixel shader for the Minimap

Texture2D<float> texHeight : register(t0);
SamplerState sampleType : register (s1);

cbuffer PerFrameConstBufPS : register(b1)
{
    float mapWidth;
    float mapHeight;
    float xRes;
    float yRes;
    float2 poi;         // selected point
    float2 poi2;        // hovered point
};

struct PixelInputType
{
float4 position : SV_POSITION;
float2 tex : TEXCOORD0;
};

// highlight point of interest (selected point), including axis guidelines
// returns true if it actually is POI
bool HighlightPoi(float2 poi, float2 tex, inout float4 color)
{
    // scale highlight
    float2 scaling = float2(1.0f / mapWidth, 1.0f / mapHeight);

    // highlight point of interest -> pure red
    if (tex.x > poi.y - scaling.y && tex.y > poi.x - scaling.x &&
            tex.x < poi.y + scaling.y && tex.y < poi.x + scaling.x)
    {
        return true;
    }
    // draw guidelines -> mix color with red
    if (tex.x > poi.y - scaling.y && tex.x < poi.y + scaling.y)
    {
        color.r = 0.5f;
    }
    if (tex.y > poi.x - scaling.x && tex.y < poi.x + scaling.x)
    {
        color.r = 0.5f;
    }

    return false;
}


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

    // highlight POIs and guidelines
    output = float4(1.0f, 0.0f, 0.0f, 1.0f);
    if (HighlightPoi(poi, tex, color) || HighlightPoi(poi2, tex, color))
    {
        return output;
    }
    else
    {
        output = float4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    // color mapping for Mandelbrot based on height value
    float height = (texHeight.Sample(sampleType, tex).r);
    height /= 10.0f;

    if (height == 0.0f)
    {
        color.rgb += float3(0.9f, 0.9f, 0.0f);
    }
    else
    {
        height = 3.0f * sqrt(height);

        color.r += height / 0.9f;
        color.g = height / 0.9f;
        color.b = height / 0.3f;
    }

    output = color * 1.0f;
    output = saturate(output);

    return output;
}