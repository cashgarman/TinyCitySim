#include "InputHandler.h"

#include "GardenTile.h"
#include "LogPanel.h"

namespace TinyCitySim
{
    InputHandler::InputHandler(TileGrid& grid)
        : grid_(grid)
    {
    }

    void InputHandler::OnMouseMove(int screenX, int screenY) noexcept
    {
        hoveredTile_ = grid_.ScreenToTile(screenX, screenY);
    }

    // This function handles mouse click events and tries to log which tile was clicked.
    void InputHandler::OnMouseClick(int screenX, int screenY)
    {
        // Call the grid_'s ScreenToTile method to convert screen coordinates (pixels) to a tile coordinate.
        // The result might not always be valid (for example, clicking outside the grid), so it's wrapped in std::optional.
        // std::optional<TileCoord> means it can either contain a valid TileCoord or nothing at all.
        const std::optional<TileCoord> tile = grid_.ScreenToTile(screenX, screenY);

        // Check if there is a valid tile coordinate value inside the optional.
        // .has_value() returns true if 'tile' actually contains a coordinate.
        if (!tile.has_value())
        {
            // If the tile coordinate is not valid (click was outside the grid), exit the function early.
            return;
        }

        // Use structured bindings (a C++17 feature) to conveniently unpack the TileCoord pair into 'col' and 'row' variables.
        // The * operator dereferences the optional, giving access to the contained TileCoord object.
        const auto [col, row] = *tile;

        // Log the tile coordinates using the logging panel's AddEntry method.
        // LogPanel::Instance() gets the single log panel instance (singleton pattern).
        // AddEntry(col, row) records which column and row were clicked.
        LogPanel::Instance().AddEntry(col, row, NameFor(grid_.At(col, row).type));
    }
}
