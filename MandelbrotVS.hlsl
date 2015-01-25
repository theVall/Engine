
//TODO
StructuredBuffer<float> bufHeight : register(t0);

SamplerState samplerHeight : register(s0);

cbuffer MatrixBuf : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
}

struct PixelInputType
{
float4 position	    : SV_POSITION;
float2 tex      	: TEXCOORD0;
float3 localPos	    : TEXCOORD1;
float3 color   : TEXCOORD2;
};


PixelInputType Main(float4 pos : POSITION, uint vid : SV_VertexID)
{
    PixelInputType output;

    float4 posLocal = float4(pos.x, 0.0f, pos.y, 1.0f);
    float2 uvLocal = pos.xy / 512.0f;

    float height = bufHeight[vid];

    posLocal.y += height * 1.0f;

    output.position = mul(posLocal, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.tex = uvLocal;
    output.color.r = height;

    return output;
}