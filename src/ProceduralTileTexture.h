#pragma once

#include "GardenTile.h"

#include <cstdint>
#include <span>

namespace TinyCitySim
{
    inline constexpr int kProceduralTileTexSize = 32;

    class ProceduralTileTexture
    {
    public:
        static void GenerateCell(
            GardenTileType type,
            int col,
            int row,
            std::uint32_t seed,
            int size,
            std::span<std::uint8_t> rgbaOut);
    };
}
