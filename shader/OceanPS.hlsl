
//  Pixel shader for ocean surface shading

// Textures
Texture2D texDisplacement : register(t0);
Texture2D texGradient : register(t1);

// Samplers
SamplerState samplerDisplacement : register(s0);
SamplerState samplerGradient : register(s1);

cbuffer PerFrameConstBufPS : register(b1)
{
    float3 eyeVec;
    float3 lightDir;
};

struct PixelInputType
{
float4 position	    : SV_POSITION;
float2 tex	        : TEXCOORD0;
float3 localPos	    : TEXCOORD1;
float3 debugColor   : TEXCOORD2;
};


// Ocean shading
float4 OceanPS(PixelInputType input) : SV_Target
{
    float3 sunDir = -normalize(lightDir);
    float4 ambientColor = float4(0.1f, 0.2f, 0.3f, 1.0f);
    float4 diffuseColor = float4(0.1f, 0.3f, 0.8f, 1.0f);

    float2 gradient = texGradient.Sample(samplerGradient, input.tex).xy;
    float folding = texGradient.Sample(samplerGradient, input.tex).w * 0.8;

    float3 normal = normalize(float3(gradient, 14.0f));

    float3 eyeDir = normalize(eyeVec - input.localPos);
    float3 reflectVec = normalize(reflect(-eyeDir, normal));

    float dotNL = max(0.0f, dot(normal, sunDir));
    float4 surfaceColor = 0.75f * ambientColor + 1.0f * diffuseColor * dotNL;

    float cosSpec = clamp(dot(reflectVec, sunDir), 0.0f, 1.0f);
    float sunStreak = pow(cosSpec, 400.0f);
    surfaceColor += float4(0.8f, 0.8f, 0.5f, 1.0f) * sunStreak;
    surfaceColor += float4(folding, folding, folding, 1.0f);

    return surfaceColor;

    //return float4(input.debugColor, 1.0f);
}

float4 WireframePS() : SV_Target
{
    return float4(0.9f, 0.9f, 0.9f, 1.0f);
}