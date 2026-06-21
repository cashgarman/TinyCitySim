#include "TileGrid.h"

namespace TinyCitySim
{
    TileGrid::TileGrid(int width, int height, int tileSize)
        : width_(width)
        , height_(height)
        , tileSize_(tileSize)
    {
    }

    void TileGrid::SetClientSize(int clientWidth, int clientHeight) noexcept
    {
        const int gridPixelWidth = width_ * tileSize_;
        const int gridPixelHeight = height_ * tileSize_;

        originX_ = (clientWidth - gridPixelWidth) / 2;
        originY_ = (clientHeight - gridPixelHeight) / 2;
    }

    std::optional<TileCoord> TileGrid::ScreenToTile(int screenX, int screenY) const noexcept
    {
        const int localX = screenX - originX_;
        const int localY = screenY - originY_;

        if (localX < 0 || localY < 0)
        {
            return std::nullopt;
        }

        const int col = localX / tileSize_;
        const int row = localY / tileSize_;

        if (col >= width_ || row >= height_)
        {
            return std::nullopt;
        }

        return TileCoord{ col, row };
    }
}
