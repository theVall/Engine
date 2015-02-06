
//  Vertex shader for line highlighting.

//  Common matrices.
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct VertexInputType
{
float4 position : POSITION;
};

struct PixelInputType
{
float4 position : SV_POSITION;
};

// Entry point main method.
PixelInputType Main(VertexInputType input)
{
    PixelInputType output;

    input.position.w = 1.0f;

    // position transformations
    output.position = mul( input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    return output;
}