#pragma once

#include "GardenTile.h"

#include <cstdint>
#include <span>

namespace TinyCitySim
{
    inline constexpr int kProceduralTileTexSize = 32;

    struct TileGenContext
    {
        GardenTileType type = GardenTileType::Lawn;
        int col = 0;
        int row = 0;
        std::uint32_t seed = 0;
        int size = kProceduralTileTexSize;
        GardenTileType north = GardenTileType::Lawn;
        GardenTileType south = GardenTileType::Lawn;
        GardenTileType east = GardenTileType::Lawn;
        GardenTileType west = GardenTileType::Lawn;
    };

    class ProceduralTileTexture
    {
    public:
        static void GenerateCell(const TileGenContext& ctx, std::span<std::uint8_t> rgbaOut);
    };
}
