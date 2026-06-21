#include "GrassSimulation.h"

#include "GardenTile.h"

#include <algorithm>

namespace TinyCitySim
{
    namespace
    {
        inline constexpr float kPassiveRegrowthPerSec = 0.04f;
        inline constexpr float kWaterBoostMultiplier = 3.0f;
        inline constexpr float kClickGrassBump = 0.12f;
        inline constexpr float kWaterBoostDuration = 2.5f;
    }

    void GrassSimulation::Update(TileGrid& grid, float dt) noexcept
    {
        if (dt <= 0.0f)
        {
            return;
        }

        for (int row = 0; row < grid.Height(); ++row)
        {
            for (int col = 0; col < grid.Width(); ++col)
            {
                GardenTile& tile = grid.MutableAt(col, row);

                if (tile.type != GardenTileType::Lawn)
                {
                    continue;
                }

                if (tile.waterBoost > 0.0f)
                {
                    tile.waterBoost = std::max(0.0f, tile.waterBoost - dt);
                }

                if (tile.grassLevel >= 1.0f)
                {
                    continue;
                }

                float regrowthRate = kPassiveRegrowthPerSec;
                if (tile.waterBoost > 0.0f)
                {
                    regrowthRate *= kWaterBoostMultiplier;
                }

                tile.grassLevel = std::min(1.0f, tile.grassLevel + regrowthRate * dt);
            }
        }
    }

    void GrassSimulation::WaterTile(TileGrid& grid, int col, int row) noexcept
    {
        GardenTile& tile = grid.MutableAt(col, row);

        if (tile.type != GardenTileType::Lawn)
        {
            return;
        }

        tile.grassLevel = std::min(1.0f, tile.grassLevel + kClickGrassBump);
        tile.waterBoost = kWaterBoostDuration;
    }
}
