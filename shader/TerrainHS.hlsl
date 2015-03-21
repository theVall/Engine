// Buffer changing on a per frame basis.
cbuffer PerFrameConstBuf : register(b1)
{
    float3 eyeVec;
    float3 tessFactor;
};

// incoming
struct HullInputType
{
float4 posWorld : POSWORLD;
float2 tex : TEXCOORD;
float3 normal : NORMAL;
float4 color : COLOR;
float4 positionModel : POSMODEL;
float vertDistFact : VERTDISTFACT;
};

// Output control point
struct ControlPointOutputType
{
float4 posWorld : POSWORLD;
float2 tex : TEXCOORD;
float3 normal : NORMAL;
float4 color : COLOR;
float4 positionModel : POSMODEL;
};

// Output patch constant data.
struct ConstantDataOutputType
{
float edgeTessFactor[3]	: SV_TessFactor;
float insideTessFactor	: SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

// Patch Constant Function
ConstantDataOutputType CalcHSPatchConstants(InputPatch<HullInputType,
                                            NUM_CONTROL_POINTS> inPatch,
                                            uint PatchID : SV_PrimitiveID)
{
    ConstantDataOutputType output;
    float4 edgeTessFact = tessFactor.xxxy;

    // Calculate edge scale factor from vertex scale factor: simply compute
    // average tess factor between the two vertices making up an edge
    edgeTessFact.x = 0.5 * (inPatch[1].vertDistFact + inPatch[2].vertDistFact);
    edgeTessFact.y = 0.5 * (inPatch[2].vertDistFact + inPatch[0].vertDistFact);
    edgeTessFact.z = 0.5 * (inPatch[0].vertDistFact + inPatch[1].vertDistFact);
    edgeTessFact.w = edgeTessFact.x;

    // Multiply them by global tessellation factor
    edgeTessFact *= float4(7.0f, 7.0f, 7.0f, 5.0f) ;// tessFactor.xxxy;

    output.edgeTessFactor[0] = edgeTessFact.x;
    output.edgeTessFactor[1] = edgeTessFact.y;
    output.edgeTessFactor[2] = edgeTessFact.z;
    output.insideTessFactor  = edgeTessFact.w;

    return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
[maxtessfactor(15.0)]
ControlPointOutputType Main(InputPatch<HullInputType, NUM_CONTROL_POINTS> inPatch,
                            uint pointId : SV_OutputControlPointID)
{
    ControlPointOutputType output;

    output.posWorld = inPatch[pointId].posWorld;
    output.tex = inPatch[pointId].tex;
    output.normal = inPatch[pointId].normal;
    output.color = inPatch[pointId].color;
    output.positionModel = inPatch[pointId].positionModel;

    return output;
}
