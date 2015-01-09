
//  Vertex shader for terrain rendering.

//  Common matrices.
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct VertexInputType
{
float3 position : POSITION;
float3 tex : TEXCOORD0;
float3 normal : NORMAL;
float4 color : COLOR;
};

struct PixelInputType
{
float4 position : SV_POSITION;
float2 tex : TEXCOORD0;
float3 normal : NORMAL;
float4 color : COLOR;
float4 positionModel : TEXCOORD1;
};

// Entry point main method.
PixelInputType Main(VertexInputType input)
{
    PixelInputType output;

    float4 positionModel = float4(input.position, 1.0);
    output.positionModel = positionModel;

    // position calculations model -> view -> projection
    output.position = mul(positionModel, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.tex = input.tex.xy;

    // normal calculations
    output.normal = mul(input.normal, (float3x3)worldMatrix);
    output.normal = normalize(output.normal);

    output.color = input.color;

    return output;
}