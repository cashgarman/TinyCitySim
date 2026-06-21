Texture2D tileAtlas : register(t0);
SamplerState tileSampler : register(s0);

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float2 tileCoord : TEXCOORD1;
    float4 color : COLOR;
    float tileType : TEXCOORD2;
};

cbuffer AtlasBuffer : register(b1)
{
    float2 atlasGridSize;
    float2 atlasInvGridSize;
    float useSolidColor;
    float cellTexSize;
    float elapsedTime;
    float atlasPadding;
};

static const float kTypeLawn = 0.0;
static const float kTypeFlowerBed = 3.0;
static const float kTypeTree = 5.0;
static const float kTypePond = 6.0;

float Hash11(float2 p)
{
    float h = dot(p, float2(127.1, 311.7));
    return frac(sin(h) * 43758.5453);
}

float4 main(PixelInput input) : SV_TARGET
{
    if (useSolidColor > 0.5f)
    {
        return input.color;
    }

    const float inset = 0.5f / cellTexSize;
    float2 localUv = lerp(
        float2(inset, inset),
        float2(1.0f - inset, 1.0f - inset),
        input.uv);

    const float2 worldPos = input.tileCoord + localUv;
    const float typeId = input.tileType;

    if (abs(typeId - kTypePond) < 0.5f)
    {
        const float rippleOffset = sin((worldPos.x * 12.0 + worldPos.y * 8.0) + elapsedTime * 2.5) * 0.012;
        localUv += float2(rippleOffset, rippleOffset * 0.6);
    }

    const float2 atlasUV = (input.tileCoord + localUv) * atlasInvGridSize;
    float4 color = tileAtlas.Sample(tileSampler, atlasUV);

    if (abs(typeId - kTypePond) < 0.5f)
    {
        const float shimmer = sin(elapsedTime * 1.8 + worldPos.x * 20.0 + worldPos.y * 14.0) * 0.04;
        color.rgb += shimmer;
    }
    else if (abs(typeId - kTypeTree) < 0.5f)
    {
        const float canopyMask = step(0.35, 1.0 - input.uv.y);
        const float rustle = sin(elapsedTime * 2.2 + worldPos.x * 16.0) * 0.035 * canopyMask;
        color.rgb += rustle;
    }
    else if (abs(typeId - kTypeFlowerBed) < 0.5f)
    {
        float sparkle = 0.0;
        for (int i = 0; i < 3; ++i)
        {
            const float2 sparkSeed = input.tileCoord + float2(0.17 * (i + 1), 0.31 * (i + 2));
            const float2 sparkPos = float2(
                Hash11(sparkSeed),
                Hash11(sparkSeed + float2(3.7, 1.9)));
            const float2 delta = (localUv - sparkPos) * cellTexSize;
            const float dist = length(delta);
            const float phase = elapsedTime * 3.0 + Hash11(sparkSeed + 9.1) * 6.28;
            sparkle += saturate(1.0 - dist / 1.8) * (sin(phase) * 0.5 + 0.5) * 0.18;
        }

        color.rgb += sparkle;
    }
    else if (abs(typeId - kTypeLawn) < 0.5f)
    {
        const float breeze = sin(elapsedTime * 1.2 + worldPos.x * 10.0 + worldPos.y * 6.0) * 0.015;
        color.rgb += breeze;
    }

    return color;
}
