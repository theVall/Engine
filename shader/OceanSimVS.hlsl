
//  Vertex shader for ocean simulation.

struct VertexInputType
{
float4 position		: POSITION;
};

struct PixelInputType
{
float4 position		: SV_POSITION;
float2 tex  		: TEXCOORD0;
};

PixelInputType Main(VertexInputType input)
{
    PixelInputType output;

    output.position = input.position;
    output.tex.x = 0.5f + input.position.x * 0.5f;
    output.tex.y = 0.5f - input.position.y * 0.5f;

    return output;
}