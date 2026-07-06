struct Vertex
{
    float4 position;
    float4 color;
    float2 uv;
};

StructuredBuffer<Vertex> Vertices : register(t0);

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

VSOutput VS(uint vertexID : SV_VertexID)
{
    Vertex vertex = Vertices[vertexID];

    VSOutput output;
    output.position = vertex.position;
    output.color = vertex.color;
    output.uv = vertex.uv;

    return output;
}