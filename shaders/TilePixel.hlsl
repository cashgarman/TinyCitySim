Texture2D tileAtlas : register(t0);
SamplerState tileSampler : register(s0);

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float2 tileCoord : TEXCOORD1;
    float4 color : COLOR;
};

cbuffer AtlasBuffer : register(b1)
{
    float2 atlasGridSize;
    float2 atlasInvGridSize;
    float useSolidColor;
    float cellTexSize;
    float2 atlasPadding;
};

float4 main(PixelInput input) : SV_TARGET
{
    if (useSolidColor > 0.5f)
    {
        return input.color;
    }

    // Avoid sampling texels at the absolute border of the atlas cell by applying a small inset, based on half a texel in normalized space.
    const float inset = 0.5f / cellTexSize; // Each atlas cell is cellTexSize texels wide/high; 0.5 gets us to the texel center, dividing normalizes to [0,1].

    // Interpolate the UV coordinate from [inset, 1-inset] instead of [0, 1], ensuring we stay within safe texel bounds for filtering.
    const float2 localUv = lerp(
        float2(inset, inset),               // Lower-left (inset,inset), avoids border
        float2(1.0f - inset, 1.0f - inset), // Upper-right (1-inset,1-inset), avoids border
        input.uv);                          // UV within cell ([0,1] range for the quad)

    // Compute the UV into the atlas: offset by tileCoord to pick the correct tile, add localUv to sample within the tile, then scale by reciprocal of atlas grid size to map to [0,1] atlas space.
    const float2 atlasUV = (input.tileCoord + localUv) * atlasInvGridSize;

    // Sample from the atlas texture using computed UVs and hardware filtering (through the sampler).
    return tileAtlas.Sample(tileSampler, atlasUV);
}
