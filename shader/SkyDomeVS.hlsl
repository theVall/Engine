
// constant buffer
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// typedefs
struct VertexInputType
{
float4 position : POSITION;
float2 texCoord : TEXCOORD;
};

struct PixelInputType
{
float4 position : SV_POSITION;
float4 domePosition : TEXCOORD0;
float2 tex : TEXCOORD1;
};


PixelInputType Main(VertexInputType input)
{
    PixelInputType output;

    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Send the unmodified position through to the pixel shader.
    output.domePosition = input.position;
    output.tex = input.texCoord;

    return output;
}