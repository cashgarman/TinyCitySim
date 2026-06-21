#pragma once

#include "TileGrid.h"

namespace TinyCitySim
{
    class GrassSimulation
    {
    public:
        void Update(TileGrid& grid, float dt) noexcept;
        void WaterTile(TileGrid& grid, int col, int row) noexcept;
    };
}
