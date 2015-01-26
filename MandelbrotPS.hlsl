
cbuffer PerFrameConstBufPS : register(b1)
{
    float3 lightDir;
    int heightMapSize;
};

struct PixelInputType
{
float4 position : SV_POSITION;
float2 tex	    : TEXCOORD0;
float3 localPos	: TEXCOORD1;
float3 color    : TEXCOORD2;
float3 normal   : NORMAL;
};


float4 Main(PixelInputType input, uint pid : SV_PrimitiveID) : SV_TARGET
{
    float height = input.color.r;
    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 output = float4(1.0f, 1.0f, 1.0f, 1.0f);

    if (height < 0.1f)
    {
        color.g = 0.07f;
        color.b = 0.3f;
    }
    else if (height > 0.9f)
    {
        color.rgb = float3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        color.r = height / 0.9f;
        color.g = height / 0.9f;
        color.b = height / 0.25f;
    }

    float3 invLightDir = -lightDir;
    float lightIntensity = saturate(dot(input.normal, invLightDir));

    if (lightIntensity > 0.0f)
    {
        output += (output*lightIntensity);
    }

    output = color * output;
    output = saturate(output);

    //output.rgb = input.normal;
    return output;
}