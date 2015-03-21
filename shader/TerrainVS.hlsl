
//  Vertex shader for terrain rendering.

//  Common matrices.
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// Buffer changing on a per frame basis.
cbuffer PerFrameConstBuf : register(b1)
{
    float3 eyeVec;
    float3 tessFactor;
};

struct VertexInputType
{
float3 position : POSITION;
float3 tex : TEXCOORD;
float3 normal : NORMAL;
float4 color : COLOR;
};

// Type going into Hull shader processing.
struct HullInputType
{
float4 posWorld : POSWORLD;
float2 tex : TEXCOORD;
float3 normal : NORMAL;
float4 color : COLOR;
float4 positionModel : POSMODEL;
float vertDistFact : VERTDISTFACT;
};

// Entry point main method.
HullInputType Main(VertexInputType input)
{
    HullInputType output;

    float4 positionModel = float4(input.position, 1.0);
    output.positionModel = positionModel;
    output.posWorld = mul(positionModel, worldMatrix);

    // position calculations model -> view -> projection
    //output.position = mul(positionModel, worldMatrix);
    //output.position = mul(output.position, viewMatrix);
    //output.position = mul(output.position, projectionMatrix);

    output.tex = input.tex.xy;

    // normal calculations
    output.normal = mul(input.normal, (float3x3)worldMatrix);
    output.normal = normalize(output.normal);

    output.color = input.color;

    const float minDist = 1.0f;
    const float maxDist = 2000.0f;

    // Calculate distance between vertex and camera,
    // and a vertex distance factor issued from it.
    float dist = distance(output.posWorld.xyz, eyeVec);
    output.vertDistFact = 1.0f - clamp(((dist - minDist) / (maxDist - minDist)),
                                       0.0f,
                                       1.0f - tessFactor.z / tessFactor.x);

    return output;
}