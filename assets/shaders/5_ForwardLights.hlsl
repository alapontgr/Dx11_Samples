// Use column major so the matrices are compatible with glm ones
#pragma pack_matrix(column_major)

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 tangent : TANGENT;
    float2 uv : TEXCOORD0;
};

struct FS_INPUT
{
    float4 pos : SV_POSITION;
    float3 posWS : POSWS;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

cbuffer FrameCB : register(b0)
{
    float4x4 view;
    float4x4 viewProj;
    float4x4 invViewProj;
    float3 lightDir;
    float pad0;
	float3 mainLightColor;
	
	// Point
	float pointLightRadius;
	float3 pointLightPos;
	float pad1;
	float3 pointLightColor;

	// Spot
	float spotLightInnerCone; // cos of angle
	float3 spotLightPos;
	float spotLightOuterCone; // cos of angle
	float3 spotLightDir;
	float spotLightInnerRadius;
	float3 spotLightColor;
	float spotLightOuterRadius;
};

cbuffer DrawcallCB : register(b1)
{
    float4x4 model;
};

SamplerState bilinearSampler : register(s0);
Texture2D tex_albedo : register(t0);

FS_INPUT mainVS(VS_INPUT input)
{
    FS_INPUT output;

    float4x4 modelViewProj = mul(viewProj, model);
    output.normal = input.normal;
    output.pos = mul(modelViewProj, float4(input.pos, 1.0f));
    output.posWS = mul(model, float4(input.pos, 1.0f)).xyz;
    output.uv = input.uv;
    return output;
}

// Obtained from Filament (https://google.github.io/filament/Filament.html#lighting/directlighting/punctuallights)
float getSquareFalloffAttenuation(float3 posToLight, float lightInvRadius) {
    float distanceSquare = dot(posToLight, posToLight);
    float factor = distanceSquare * lightInvRadius * lightInvRadius;
    float smoothFactor = max(1.0 - factor * factor, 0.0);
    return (smoothFactor * smoothFactor) / max(distanceSquare, 1e-4);
}

float getSpotAngleAttenuation(float3 l, float3 lightDir, float innerAngle, float outerAngle) 
{
    // the scale and offset computations can be done CPU-side
    float cosOuter = cos(outerAngle);
    float spotScale = 1.0 / max(cos(innerAngle) - cosOuter, 1e-4);
    float spotOffset = -cosOuter * spotScale;

    float cd = dot(normalize(-lightDir), l);
    float attenuation = clamp(cd * spotScale + spotOffset, 0.0, 1.0);
    return attenuation * attenuation;
}

float4 mainFS(FS_INPUT input) : SV_Target
{
    float3 albedo = tex_albedo.Sample(bilinearSampler, input.uv).rgb;

    // Ambient lighting (just to make sure we see something in non lit areas
    float3 ambient = float3(1.0f, 1.0f, 1.0f) * 0.1f;

    float3 lighting = float3(0.0f, 0.0f, 0.0f);
    // Dir light
    float NdotL = max(dot(input.normal, -lightDir), 0.0f);
    lighting += NdotL * mainLightColor;

    // Point light
    float3 posToPoint = pointLightPos - input.posWS;
    float3 LPoint = normalize(posToPoint);
    NdotL = max(0.0f, dot(LPoint, input.normal));
    float attenPoint = getSquareFalloffAttenuation(posToPoint, 1.0f / pointLightRadius);
    lighting += attenPoint * NdotL * pointLightColor;

    float3 color = albedo * (ambient + lighting);
    return float4(color, 1.0f);
}