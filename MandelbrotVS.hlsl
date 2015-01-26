
StructuredBuffer<float> bufHeight : register(t0);

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


PixelInputType Main(float2 pos : POSITION, uint vid : SV_VertexID)
{
    // TODO: get that into a constant!
    uint heightmapDim = 2048;

    PixelInputType output;

    // TODO: scaling factors in const buffer
    float4 posLocal = float4(pos.x * 1.0f, 0.0f, pos.y * 1.0f, 1.0f);
    float2 uvLocal = pos.xy / heightmapDim;

    float height = bufHeight[uvLocal.x * heightmapDim * heightmapDim + uvLocal.y * heightmapDim];

    posLocal.y += height * 100.0f;

    output.position = mul(posLocal, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.localPos = posLocal.xyz;
    output.tex = uvLocal;
    output.color = float3(height, 0.0f, 0.0f);

    return output;
}