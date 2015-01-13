
//  Pixel shader for terrain coloring and lighting

// TODO: TexArray
Texture2D sandTex : register(t0);
Texture2D rockTex : register(t1);
Texture2D mossyRockTex : register(t2);
Texture2D grassTex : register(t3);
Texture2D snowTex : register(t4);

Texture2DArray textures;

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

    // TODO texArray
    float4 sandTexColor = sandTex.Sample(SampleType, input.tex);
    float4 rockTexColor = rockTex.Sample(SampleType, input.tex);
    float4 mossyRockTexColor = mossyRockTex.Sample(SampleType, input.tex);
    float4 grassTexColor = grassTex.Sample(SampleType, input.tex);
    float4 snowTexColor = snowTex.Sample(SampleType, input.tex);

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
        blendFactor = abs(height) / scaling;
        textureColor = lerp(rockTexColor, sandTexColor, blendFactor);
    }
    else if (height < scaling)
    {
        blendFactor = abs(height) / scaling;
        textureColor = lerp(rockTexColor, mossyRockTexColor, blendFactor);
    }
    else
    {
        if (slope < 0.2f)
        {
            blendFactor = slope / 0.3f;
            textureColor = lerp(grassTexColor, mossyRockTexColor, blendFactor);
        }
        if ((slope < 0.7) && (slope >= 0.2f))
        {
            blendFactor = (slope - 0.2f) * (1.0f / (0.7f - 0.2f));
            if (height <= 500.0f)
            {
                textureColor = lerp(mossyRockTexColor, rockTexColor, blendFactor);
            }
            else
            {
                textureColor = lerp(snowTexColor, rockTexColor, blendFactor);
            }
        }
    }

    if (height > 400.0f)
    {
        blendFactor = (height - 400.0f) / 50.0f;
        textureColor = lerp(textureColor, rockTexColor, blendFactor);
    }
    if (height > 450.0f)
    {
        textureColor = rockTexColor;
    }
    if (height > 500.0f && slope < 0.2)
    {
        blendFactor = saturate((height - 500.0f) / 50.0f);
        textureColor = lerp(rockTexColor, snowTexColor, blendFactor);;
    }


    lightDir = -lightDirection;
    lightIntensity = saturate(dot(input.normal, lightDir));

    if (lightIntensity > 0.0f)
    {
        color += (diffuseColor*lightIntensity);
    }

    color = color *textureColor;

    // combine with color map value
    color = saturate(color);

    //lerp(color, input.color, float4(0.5f, 0.5f, 0.5f, 0.5f));

    color.w = 1.0;
    // DEBUG ONLY
    //color = float4(1,1,1,1);

    return color;
}
