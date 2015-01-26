
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
};


float4 Main(PixelInputType input, uint pid : SV_PrimitiveID) : SV_TARGET
{
    float height = input.color.r;
    float4 output = float4(0.0f, 0.0f, 0.0f, 1.0f);

    if (height < 0.1f)
    {
        output.b = 0.3f;
        return output;
    }
    else
    {
        output.b = height / 0.9f;
    }
    output.r = height;

    return output;
}