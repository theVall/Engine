
//  Pixel shader for line highlighting

struct PixelInputType
{
float4 position : SV_POSITION;
};


//  Entry point main method.
float4 Main(PixelInputType input) : SV_TARGET
{
    float4 lineColor = float4(1.0f, 0.0f, 0.0f, 1.0f);

    return lineColor;
}