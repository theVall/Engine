
StructuredBuffer<float> bufHeight : register(t0);

cbuffer MatrixBuf : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
}

cbuffer PerFrameConstBuf : register(b1)
{
    float heightMapDim;
    float xScale;
    float yScale;
    float heigthScale;
}

struct PixelInputType
{
float4 position	    : SV_POSITION;
float2 tex      	: TEXCOORD0;
float3 localPos	    : TEXCOORD1;
float3 color        : TEXCOORD2;
float3 normal       : NORMAL;
};


PixelInputType Main(float2 pos : POSITION, uint vid : SV_VertexID)
{
    PixelInputType output;

    float4 posLocal = float4(pos.x * xScale, 0.0f, pos.y * yScale, 1.0f);
    float2 uvLocal = pos.xy / heightMapDim;

    uint index = uvLocal.x * heightMapDim * heightMapDim + uvLocal.y * heightMapDim;

    float height = sqrt(bufHeight[index]);
    output.color = float3((height*height) / 10000.f, 0.0f, 0.0f);

    posLocal.y += height * heigthScale;

    output.position = mul(posLocal, worldMatrix);

    // normal calculation
    float4 posLocalN1 = float4((pos.x + 1.0f) * xScale, 0.0f, pos.y * yScale, 1.0f);
    posLocalN1.y = bufHeight[index + 1] * heigthScale;
    posLocalN1 = mul(posLocalN1, worldMatrix);

    float4 posLocalN2 = float4(pos.x * xScale, 0.0f, (pos.y + 1.0f) * yScale, 1.0f);
    posLocalN2.y = bufHeight[index + heightMapDim] * heigthScale;
    posLocalN2 = mul(posLocalN2, worldMatrix);

    float4 vec1 = posLocalN1 - output.position;
    float4 vec2 = posLocalN1 - posLocalN2;
    output.normal = normalize(cross(vec1.xyz, vec2.xyz));

    // view-proj transformation
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.localPos = posLocal.xyz;
    output.tex = uvLocal;

    return output;
}