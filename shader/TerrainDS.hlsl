// perlin noise texture
Texture2D noiseTex : register(t0);

SamplerState sampleLinear : register(s0);

//  Common matrices.
cbuffer MatrixBuffer : register(b0)
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

// incoming control point data
struct DomainInputControlPointType
{
float4 posWorld : POSWORLD;
float2 tex : TEXCOORD;
float3 normal : NORMAL;
float4 color : COLOR;
float4 positionModel : POSMODEL;
float vertDistFact : VERTDISTFACT;
};
// incoming constant data
struct DomainInputConstantDataType
{
float edgeTessFactor[3]	: SV_TessFactor;
float insideTessFactor : SV_InsideTessFactor;
};

// outgoing
struct PixelInputType
{
float4 position : SV_POSITION;
float2 tex : TEXCOORD;
float3 normal : NORMAL;
float4 color : COLOR;
float4 positionModel : POSMODEL;
float vertDistFact : VERTDISTFACT;
};

#define NUM_CONTROL_POINTS 3
// triangles
[domain("tri")]
PixelInputType Main(DomainInputConstantDataType input,
                    float3 domain : SV_DomainLocation,
                    const OutputPatch<DomainInputControlPointType, NUM_CONTROL_POINTS> patch)
{
    PixelInputType output;

    float3 worldPos = float3(patch[0].posWorld.xyz * domain.x +
                             patch[1].posWorld.xyz * domain.y +
                             patch[2].posWorld.xyz * domain.z);
    float3 normal = float3(patch[0].normal * domain.x +
                           patch[1].normal * domain.y +
                           patch[2].normal * domain.z);
    output.normal = normalize(normal);

    output.tex = float2(patch[0].tex * domain.x +
                        patch[1].tex * domain.y +
                        patch[2].tex * domain.z);

    //// Calculate MIP level to fetch normal from
    //float heightMapMIPLevel = clamp((distance(worldPos, eyeVec) - 100.0f) / 100.0f, 0.0f, 3.0f);

    //// Sample normal and height map
    //float4 normalHeight = g_nmhTexture.SampleLevel(g_samLinear, output.tex, heightMapMIPLevel);

    //// Displace vertex along normal
    //worldPos += normal * (detailTessHeightScale * (normalHeight.w - 1.0f));

    float4 noise = noiseTex.SampleLevel(sampleLinear, frac(normal.xz), 0);
    worldPos.y += noise.r * 10.0f;

    output.positionModel = float4(worldPos, 1.0f);

    // Transform world position with view-projection matrix
    output.position = mul(float4(worldPos, 1.0f), viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.vertDistFact = patch[0].vertDistFact;

    return output;
}
