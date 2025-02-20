
//  Pixel shader for terrain coloring and lighting

// TODO: TexArray
Texture2D sandTex : register(t1);
Texture2D rockTex : register(t2);
Texture2D mossyRockTex : register(t3);
Texture2D grassTex : register(t4);
Texture2D snowTex : register(t5);

Texture2DArray textures;

SamplerState sampleLinear : register(s0);

cbuffer PerFrameConstBuf : register(b1)
{
    float3 eyeVec;
    float3 tessFactor;
};

cbuffer LightBuffer : register(b2)
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
    float scaling;
};

struct PixelInputType
{
float4 position : SV_POSITION;
float2 tex : TEXCOORD;
float3 normal : NORMAL;
float4 color : COLOR;
float4 positionModel : POSMODEL;
float vertDistFact : VERTDISTFACT;
};


// primitive simulation of non-uniform atmospheric fog
float3 CalcFogColor(float3 pixelToLightVec, float3 pixelToEyeVec)
{
    return lerp(float3(0.6, 0.6, 0.7), float3(1.0, 1.1, 1.4), 0.5*dot(pixelToLightVec, -pixelToEyeVec) + 0.5);
}


// Entry point main method
float4 Main(PixelInputType input) : SV_TARGET
{
    float3 lightDir;
    float lightIntensity;
    float4 color = ambientColor;
    float4 textureColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float blendFactor;

    // TODO: variable -> cbuffer/GUI
    float fogDensity = 1.0f / 10000.0f;

    // TODO texArray
    float4 sandTexColor = sandTex.SampleLevel(sampleLinear, input.tex, 0);
    float4 rockTexColor = rockTex.SampleLevel(sampleLinear, input.tex, 0);
    float4 mossyRockTexColor = mossyRockTex.SampleLevel(sampleLinear, input.tex, 0);
    float4 grassTexColor = grassTex.SampleLevel(sampleLinear, input.tex, 0);
    float4 snowTexColor = snowTex.SampleLevel(sampleLinear, input.tex, 0);

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
            blendFactor = saturate(slope / 0.1f);
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
        color += diffuseColor*lightIntensity*1.0f;
    }

    color = color * textureColor;

    // combine with color map value
    color = saturate(color);

    // colormap is not used atm
    //lerp(color, input.color, float4(0.5f, 0.5f, 0.5f, 0.5f));

    // apply fog
    float3 pixelPos = input.positionModel.xyz;
    float3 pixelToLightVec = normalize(lightDirection - pixelPos);
    float3 pixelToEyeVec = normalize(eyeVec - pixelPos);

    color.rgb = lerp(CalcFogColor(pixelToLightVec, pixelToEyeVec).rgb,
                     color.rgb,
                     min(1.0f, exp(-length(eyeVec - pixelPos)*fogDensity)));
    color.a = length(eyeVec - pixelPos);

    return color;
}
