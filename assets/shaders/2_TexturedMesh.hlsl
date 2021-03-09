// Use column major so the matrices are compatible with glm ones
#pragma pack_matrix(column_major)

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct FS_INPUT
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float2 uv : TEXCOORD0;
};

cbuffer FrameCB : register(b0)
{
    float4x4 view;
    float4x4 viewProj;
    float4x4 invViewProj;
    float2 uvOffset;
    float2 uvScale;
};

cbuffer DrawcallCB : register(b1)
{
    float4x4 model;
};

SamplerState bilinearSampler : register(s0);
Texture2D colorTexture : register(t0);

FS_INPUT mainVS(VS_INPUT input)
{
    FS_INPUT output;

    float4x4 modelViewProj = mul(viewProj, model);
    output.color = input.normal * 0.5 + 0.5;
    output.pos = mul(modelViewProj, float4(input.pos, 1.0f));
    output.uv = input.uv;
    return output;
}

float4 mainFS(FS_INPUT input) : SV_Target
{
    return colorTexture.Sample(bilinearSampler, input.uv * uvScale + uvOffset);
}