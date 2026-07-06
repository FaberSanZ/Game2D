Texture2D DiffuseTexture : register(t0);
SamplerState DiffuseSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

float4 PS(PSInput input) : SV_TARGET
{
    float4 textureColor = DiffuseTexture.Sample(DiffuseSampler, input.uv);
    return textureColor * input.color;
}