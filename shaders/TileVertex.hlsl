struct VertexInput
{
    float2 position : POSITION;
    float2 uv : TEXCOORD0;
    float2 tileCoord : TEXCOORD1;
    float4 color : COLOR;
    float tileType : TEXCOORD2;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float2 tileCoord : TEXCOORD1;
    float4 color : COLOR;
    float tileType : TEXCOORD2;
};

cbuffer TransformBuffer : register(b0)
{
    float4x4 projection;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.position = mul(projection, float4(input.position, 0.0f, 1.0f));
    output.uv = input.uv;
    output.tileCoord = input.tileCoord;
    output.color = input.color;
    output.tileType = input.tileType;
    return output;
}
