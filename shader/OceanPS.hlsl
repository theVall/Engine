
//  Pixel shader for ocean surface shading

// Textures
Texture2D texDisplacement : register(t0);
Texture2D texGradient : register(t1);
Texture2D texSkyDome : register(t2);

// Samplers
SamplerState samplerDisplacement : register(s0);
SamplerState samplerGradient : register(s1);
SamplerState samplerSkyDome : register(s2);

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
    float4 waterMixColor = float4(0.09f, 0.18f, 0.36f, 1.0f);
    float4 skyMixColor = float4(0.4f, 0.5f, 0.6f, 1.0f);
    float4 sunColor = float4(0.8f, 0.8f, 0.2f, 1.0f);

    float3 sunDir = normalize(lightDir);
    float2 gradient = texGradient.Sample(samplerGradient, input.tex).xy;
    float folding = texGradient.Sample(samplerGradient, input.tex).w * 0.8;

    float3 normal = normalize(float3(gradient.x, 7.8f, gradient.y));

    float3 eyeDir = normalize(eyeVec - input.localPos.xyz);
    float3 reflectVec = normalize(reflect(-eyeDir, normal));
    //reflectVec.y = max(0.0, reflectVec.y);

    // calculate texture coords for sampling from sky dome texture
    float denom = 2.0f * sqrt(reflectVec.x *reflectVec.x + reflectVec.y *reflectVec.y + (reflectVec.z + 1) * (reflectVec.z + 1));
    float2 skyTexCoords = float2(reflectVec.x / denom + 0.75f, reflectVec.y / denom + 0.5f);

    float4 reflection = texSkyDome.Sample(samplerSkyDome, skyTexCoords);
    reflection.rgb = lerp(skyMixColor.rgb, reflection.rgb, 0.5f);

    float4 surfaceColor = lerp(waterMixColor, reflection, 0.25f);
    surfaceColor.rgb += float3(folding, folding, folding) * 0.1;

    float specular = pow(saturate(dot(reflectVec, sunDir)), 250.0f);
    surfaceColor += sunColor * specular;

    surfaceColor.a = 1.0f;
    //return float4(reflection.rgb, 1);
    return surfaceColor;
}

float4 WireframePS() : SV_Target
{
    return float4(0.9f, 0.9f, 0.9f, 1.0f);
}