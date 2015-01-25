
//StructuredBuffer<float> bufHeight : register(t0);

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
    //return float4(bufHeight[pid], 0.0f, 0.0f, 1.0f);

    return float4(input.color.r, 0.3f, 0.3f, 1.0f);
}