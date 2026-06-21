#pragma once

#include "TileGrid.h"

#include <optional>

namespace TinyCitySim
{
    class InputHandler
    {
    public:
        explicit InputHandler(TileGrid& grid);

        void OnMouseMove(int screenX, int screenY) noexcept;
        void OnMouseClick(int screenX, int screenY);

        [[nodiscard]] const std::optional<TileCoord>& HoveredTile() const noexcept
        {
            return hoveredTile_;
        }

    private:
        TileGrid& grid_;
        std::optional<TileCoord> hoveredTile_;
    };
}
