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
};

cbuffer DrawcallCB : register(b1)
{
    float4x4 model;
};

SamplerState bilinearSampler : register(s0);
Texture2D albedo : register(t0);

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
    float3 color = albedo.Sample(bilinearSampler, input.uv).rgb;
    float NdotL = dot(input.normal, -lightDir);
    color *= NdotL;
    return float4(color, 1.0f);
}