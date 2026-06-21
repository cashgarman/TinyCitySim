Texture2D tileAtlas : register(t0);
SamplerState tileSampler : register(s0);

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float2 tileCoord : TEXCOORD1;
    float4 color : COLOR;
    float tileType : TEXCOORD2;
    float grassLevel : TEXCOORD3;
    float waterBoost : TEXCOORD4;
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
    // Early exit: If solid color override is enabled, simply output the input color
    if (useSolidColor > 0.5f)
    {
        return input.color;
    }

    // Calculate a slightly inset (shrunk) UV to avoid texture bleeding at edges
    const float inset = 0.5f / cellTexSize;
    float2 localUv = lerp(
        float2(inset, inset),
        float2(1.0f - inset, 1.0f - inset),
        input.uv);

    // Compute world position within the grid tile (used for animation effects)
    const float2 worldPos = input.tileCoord + localUv;
    const float typeId = input.tileType;

    // POND TILE EFFECTS: Add subtle animated ripples for pond-type tiles
    if (abs(typeId - kTypePond) < 0.5f)
    {
        const float rippleOffset = sin((worldPos.x * 12.0 + worldPos.y * 8.0) + elapsedTime * 2.5) * 0.012;
        localUv += float2(rippleOffset, rippleOffset * 0.6);
    }

    // Convert local UV to atlas UV coordinates and sample the color from the atlas texture
    const float2 atlasUV = (input.tileCoord + localUv) * atlasInvGridSize;
    float4 color = tileAtlas.Sample(tileSampler, atlasUV);

    // TILE TYPE OVERLAYS: Add final animated or blending for each tile type
    if (abs(typeId - kTypePond) < 0.5f)
    {
        // Extra shimmer overlay animation for pond surface
        const float shimmer = sin(elapsedTime * 1.8 + worldPos.x * 20.0 + worldPos.y * 14.0) * 0.04;
        color.rgb += shimmer;
    }
    else if (abs(typeId - kTypeTree) < 0.5f)
    {
        // Swaying (rustle) effect for tree canopy, only applied to top of sprite (based on UV)
        const float canopyMask = step(0.35, 1.0 - input.uv.y);
        const float rustle = sin(elapsedTime * 2.2 + worldPos.x * 16.0) * 0.035 * canopyMask;
        color.rgb += rustle;
    }
    else if (abs(typeId - kTypeFlowerBed) < 0.5f)
    {
        // Dynamic sparkles overlay for flowerbed tiles: jittery and animated by time
        float sparkle = 0.0;
        for (int i = 0; i < 3; ++i)
        {
            // Use hash-based pseudo-random locations for spark positions
            const float2 sparkSeed = input.tileCoord + float2(0.17 * (i + 1), 0.31 * (i + 2));
            const float2 sparkPos = float2(
                Hash11(sparkSeed),
                Hash11(sparkSeed + float2(3.7, 1.9)));
            const float2 delta = (localUv - sparkPos) * cellTexSize;
            const float dist = length(delta);
            // Animate sparkle pulse phase with time and some unique seed per sparkle
            const float phase = elapsedTime * 3.0 + Hash11(sparkSeed + 9.1) * 6.28;
            // Composite sparkle intensity for each close-enough spark, fade out with distance
            sparkle += saturate(1.0 - dist / 1.8) * (sin(phase) * 0.5 + 0.5) * 0.18;
        }

        color.rgb += sparkle;
    }
    else if (abs(typeId - kTypeLawn) < 0.5f)
    {
        // LAWN TILE: Blend between "dirt" and grass texture based on grassLevel, with subtle breeze and wetness effects

        // Blend base dirt color and grass based on grass level
        const float grass = saturate(input.grassLevel);
        const float3 dirtColor = float3(0.38, 0.28, 0.16);
        color.rgb = lerp(dirtColor, color.rgb, grass);

        // Subtle animated breeze effect: modulates color based on world position and time
        const float breeze = sin(elapsedTime * 1.2 + worldPos.x * 10.0 + worldPos.y * 6.0) * 0.015 * grass;
        color.rgb += breeze;

        // If water boost is active, overlay a green "pulse" to suggest lush/wet grass
        if (input.waterBoost > 0.0f)
        {
            const float pulse = sin(elapsedTime * 4.0 + worldPos.x * 8.0) * 0.03 + 0.03;
            color.rgb += float3(0.0, pulse, 0.0) * grass;
        }
    }

    // Output the final color for this pixel
    return color;
}
