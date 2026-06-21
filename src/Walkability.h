#pragma once

#include "GardenTile.h"

namespace TinyCitySim
{
    [[nodiscard]] inline bool IsWalkable(GardenTileType type) noexcept
    {
        switch (type)
        {
        case GardenTileType::Lawn:
        case GardenTileType::Path:
        case GardenTileType::Patio:
            return true;
        default:
            return false;
        }
    }
}
