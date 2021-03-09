// Use column major so the matrices are compatible with glm ones
#pragma pack_matrix(column_major)

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

struct FS_INPUT
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
};

cbuffer CameraMatrices : register(b0)
{
    float4x4 view;
    float4x4 viewProj;
    float4x4 modelViewProj;
    float4x4 invViewProj;
};

float4 convertCSCoordsGLToDX(float4 dxCoordCS) 
{
    return  dxCoordCS;
}

FS_INPUT mainVS(VS_INPUT input)
{
    FS_INPUT output;
    output.color = input.normal * 0.5 + 0.5;
    output.pos = mul(modelViewProj, float4(input.pos, 1.0f));
    return output;
}

float4 mainFS(FS_INPUT input) : SV_Target
{
    return float4(input.color, 1.0f);
}