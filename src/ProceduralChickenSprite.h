#pragma once

#include <cstdint>
#include <span>

namespace TinyCitySim
{
    inline constexpr int kChickenFrameSize = 16;
    inline constexpr int kChickenFrameCount = 5;
    inline constexpr int kChickenAtlasWidth = kChickenFrameSize * kChickenFrameCount;

    class ProceduralChickenSprite
    {
    public:
        static void GenerateAtlas(std::uint32_t seed, std::span<std::uint8_t> rgbaOut);
    };
}
