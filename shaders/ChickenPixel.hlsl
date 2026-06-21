Texture2D chickenAtlas : register(t0);
SamplerState chickenSampler : register(s0);

cbuffer SpriteBuffer : register(b0)
{
    float frameCount;
    float frameWidth;
    float atlasWidth;
    float spritePadding;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PixelInput input) : SV_TARGET
{
    const float4 color = chickenAtlas.Sample(chickenSampler, input.uv);
    if (color.a < 0.05f)
    {
        discard;
    }

    return color;
}
