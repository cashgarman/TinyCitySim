struct VertexInput
{
    float2 position : POSITION;
    float4 color : COLOR;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

cbuffer TransformBuffer : register(b0)
{
    float4x4 projection;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.position = mul(projection, float4(input.position, 0.0f, 1.0f));
    output.color = input.color;
    return output;
}
