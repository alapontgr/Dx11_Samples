struct VS_INPUT
{
    uint vertexID : SV_VertexID;
};

struct FS_INPUT
{
    float4 pos : SV_POSITION;
    float3 vertexColor : COLOR0;
};

// Dx11 clip boundaries:
// - x,y values in the range {-1.0, 1.0}. Top left {-1.0, 1.0}
// - z (can be configured with the viewport). Default: {0.0, 1.0}
static const float4 s_Pos [3] =
{
    float4(0.0f, 0.5f, 0.0f, 1.0f), // Top
    float4(-0.5f, -0.5f, 0.0f, 1.0f), // Bottom left
    float4(0.5f, -0.5f, 0.0f, 1.0f) // Bottom right
};

static const float3 s_Colors [3] =
{
    float3(1.0f, 0.0f, 0.0f),
    float3(0.0f, 1.0f, 0.0f),
    float3(0.0f, 0.0f, 1.0f)
};

FS_INPUT mainVS(VS_INPUT input)
{
    FS_INPUT output;
    output.vertexColor = s_Colors[input.vertexID];
    output.pos = s_Pos[input.vertexID];
    return output;
}

float4 mainFS(FS_INPUT input) : SV_Target
{
    return float4(input.vertexColor, 1.0f);
}