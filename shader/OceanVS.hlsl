
//  Vertex shader for ocean generation from displacement (height-)map.

// Textures
Texture2D texDisplacement : register(t0);
Texture2D texGradient : register(t1);
Texture2D texSkyDome : register(t2);

// Samplers
SamplerState samplerDisplacement : register(s0);
SamplerState samplerGradient : register(s1);
SamplerState samplerSkyDome : register(s2);

cbuffer MatrixBuf : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
}

cbuffer PerFrameConstBufVS : register(b1)
{
    int numInstances;
    //float3 padding;
};

struct PixelInputType
{
float4 position	    : SV_POSITION;
float2 tex      	: TEXCOORD0;
float3 localPos	    : TEXCOORD1;
float3 debugColor   : TEXCOORD2;
};

PixelInputType Main(float2 pos : POSITION, uint instanceId : SV_InstanceID)
{
    PixelInputType output;
    int tileDiv = numInstances;
    // tile offset
    float3 offset = float3((instanceId % tileDiv) * 512.0f,
                           0.0f,
                           (instanceId / tileDiv) * 512.0f);

    float4 posLocal = float4(pos.x, 0.0f, pos.y, 1.0f);
    float2 uvLocal = pos.xy / 512.0f + 0.00976f;

    float3 displacement = texDisplacement.SampleLevel(samplerDisplacement,
                                                      uvLocal,
                                                      0.0f).xyz;
    // z in displacement map is y in object coordinates
    posLocal.xyz += displacement.xzy * 1.0f;

    output.position = mul(posLocal, worldMatrix);
    output.position.xyz += offset;
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.localPos = posLocal.xyz + offset;
    output.tex        = uvLocal;
    output.debugColor = displacement.xzy;

    return output;
}