//#pragma pack_matrix(row_major)

struct Vertex
{
    float4 position;
    float4 color;
    float2 uv;
};

StructuredBuffer<Vertex> Vertices : register(t0);

cbuffer CameraBuffer : register(b0)
{
    float4x4 viewProjectionMatrix;
};

struct InstacingData
{
    float4x4 model;
};

StructuredBuffer<InstacingData> InstanceData : register(t1);

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

VSOutput VS(uint vertexID : SV_VertexID, uint instID : SV_InstanceID)
{
    Vertex vertex = Vertices[vertexID];
    float4 meshPosition = mul(vertex.position, InstanceData[instID].model);

    VSOutput output;
    
    output.position = mul(meshPosition, viewProjectionMatrix);
    output.color = vertex.color;
    output.uv = vertex.uv;

    return output;
}