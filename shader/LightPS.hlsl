
//  Pixel shader for lightened surfaces

Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 ambientColor;
    float4 diffuseColor;
    float4 specularColor;
    float3 lightDirection;
    float  specularPower;
};

struct PixelInputType
{
float4 position : SV_POSITION;
float2 tex : TEXCOORD0;
float3 normal : NORMAL;
float3 viewDirection : TEXCOORD1;
};

// Entry point main method
float4 Main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;
    float3 reflection;
    float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

    textureColor = shaderTexture.Sample(SampleType, input.tex);
    color = ambientColor;

    lightDir = -lightDirection;
    lightIntensity = saturate(dot(input.normal, lightDir));

    if (lightIntensity > 0.0f)
    {
        color += (diffuseColor*lightIntensity);
        color = saturate(color);
        reflection = normalize(2.0f*lightIntensity*input.normal - lightDir);
        specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower);
    }

    color = color * textureColor;
    color = saturate(color + specular);

    return color;
}
