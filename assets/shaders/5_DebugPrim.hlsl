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
	float pointLightInnerRadius;
	float3 pointLightPos;
	float pointLightOuterRadius;
	float3 pointLightColor;

	// Spot
	float spotLightInnerCone; // cos of angle
	float3 spotLightPos;
	float spotLightOuterCone; // cos of angle
	float3 spotLightDir;
	float pad1;
	float3 spotLightColor;
	float pad2;
};

cbuffer DrawcallCB : register(b1)
{
    float4x4 model;
};

FS_INPUT mainVS(VS_INPUT input)
{
    FS_INPUT output;

    float4x4 modelViewProj = mul(viewProj, model);
    output.normal = input.normal;
    output.pos = mul(modelViewProj, float4(input.pos, 1.0f));
    output.uv = input.uv;
    return output;
}

float4 mainFS(FS_INPUT input) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}