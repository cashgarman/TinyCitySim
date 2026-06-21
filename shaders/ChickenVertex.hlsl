struct VertexInput
{
    float2 position : POSITION;
    float2 uv : TEXCOORD0;
    float flipX : TEXCOORD1;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

cbuffer TransformBuffer : register(b0)
{
    float4x4 projection;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.position = mul(projection, float4(input.position, 0.0f, 1.0f));

    if (input.flipX < 0.0f)
    {
        output.uv = float2(1.0f - input.uv.x, input.uv.y);
    }
    else
    {
        output.uv = input.uv;
    }

    return output;
}
