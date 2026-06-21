#include "InputHandler.h"

#include "GardenTile.h"
#include "LogPanel.h"

#include <format>

namespace TinyCitySim
{
    InputHandler::InputHandler(TileGrid& grid, GrassSimulation& grassSimulation)
        : grid_(grid)
        , grassSimulation_(grassSimulation)
    {
    }

    void InputHandler::OnMouseMove(int screenX, int screenY) noexcept
    {
        hoveredTile_ = grid_.ScreenToTile(screenX, screenY);
    }

    void InputHandler::OnMouseClick(int screenX, int screenY)
    {
        const std::optional<TileCoord> tile = grid_.ScreenToTile(screenX, screenY);

        if (!tile.has_value())
        {
            return;
        }

        const auto [col, row] = *tile;
        grassSimulation_.WaterTile(grid_, col, row);

        const GardenTile& clickedTile = grid_.At(col, row);
        const int grassPercent = static_cast<int>(clickedTile.grassLevel * 100.0f + 0.5f);
        const std::wstring message = std::format(
            L"{} (grass {}%)",
            NameFor(clickedTile.type),
            grassPercent);

        LogPanel::Instance().AddEntry(col, row, message);
    }
}
