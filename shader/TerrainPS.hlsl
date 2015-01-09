
//  Pixel shader for terrain coloring and lighting

Texture2D sandTex : register(t0);
Texture2D rockTex : register(t1);
Texture2D mossyRockTex : register(t2);
Texture2D grassTex : register(t3);

SamplerState SampleType;

cbuffer LightBuffer
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
    float scaling;
};

struct PixelInputType
{
float4 position : SV_POSITION;
float2 tex : TEXCOORD0;
float3 normal : NORMAL;
float4 color : COLOR;
float4 positionModel : TEXCOORD1;
};

// Entry point main method
float4 Main(PixelInputType input) : SV_TARGET
{
    float3 lightDir;
    float lightIntensity;
    float4 color = ambientColor;
    float4 textureColor;
    float blendFactor;

    float4 sandTexColor = sandTex.Sample(SampleType, input.tex);
    float4 rockTexColor = rockTex.Sample(SampleType, input.tex);
    float4 mossyRockTexColor = mossyRockTex.Sample(SampleType, input.tex);
    float4 grassTexColor = grassTex.Sample(SampleType, input.tex);

    // Calculate the slope of this point.
    float slope = 1.0f - input.normal.y;
    // Calculate the height.
    float height = input.positionModel.y;

    // Determine which texture to use based on slope and height.
    if (slope >= 0.7f)
    {
        textureColor = rockTexColor;
    }
    else if (height < -scaling)
    {
        textureColor = sandTexColor;
    }
    else if (height < 0.0f)
    {
        blendFactor = abs(height)/scaling;
        textureColor = lerp(grassTexColor, sandTexColor, blendFactor);;
    }
    else
    {
        if (slope < 0.3f)
        {
            blendFactor = slope / 0.3f;
            textureColor = lerp(grassTexColor, mossyRockTexColor, blendFactor);
        }
        if ((slope < 0.7) && (slope >= 0.3f))
        {
            blendFactor = (slope - 0.3f) * (1.0f / (0.7f - 0.3f));
            textureColor = lerp(mossyRockTexColor, rockTexColor, blendFactor);
        }
    }

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
