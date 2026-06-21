#pragma once

#include "GrassSimulation.h"
#include "TileGrid.h"

#include <optional>

namespace TinyCitySim
{
    class InputHandler
    {
    public:
        InputHandler(TileGrid& grid, GrassSimulation& grassSimulation);

        void OnMouseMove(int screenX, int screenY) noexcept;
        void OnMouseClick(int screenX, int screenY);

        [[nodiscard]] const std::optional<TileCoord>& HoveredTile() const noexcept
        {
            return hoveredTile_;
        }

    private:
        TileGrid& grid_;
        GrassSimulation& grassSimulation_;
        std::optional<TileCoord> hoveredTile_;
    };
}
