struct Vertex
{
    float4 Position;
    float4 Color;
};

StructuredBuffer<Vertex> Vertices : register(t0);


struct PixelInputType
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

PixelInputType VS(uint vId : SV_VertexID)
{
    
    Vertex input = Vertices[vId];
    
    PixelInputType output;
    output.Pos = input.Position;
    output.Color = input.Color;
    
    return output;
}