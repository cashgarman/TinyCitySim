#include "TileGrid.h"

namespace TinyCitySim
{
    TileGrid::TileGrid(int width, int height, int tileSize)
        : width_(width)
        , height_(height)
        , tileSize_(tileSize)
    {
        // Row-major layout: row 0 is the top row; each row occupies width_ consecutive slots.
        tiles_.resize(static_cast<size_t>(width * height));
    }

    bool TileGrid::InBounds(int col, int row) const noexcept
    {
        return col >= 0 && col < width_ && row >= 0 && row < height_;
    }

    const GardenTile& TileGrid::At(int col, int row) const noexcept
    {
        static const GardenTile kOutOfBounds{};

        if (!InBounds(col, row))
        {
            return kOutOfBounds;
        }

        const size_t index = static_cast<size_t>(row * width_ + col);
        return tiles_[index];
    }

    void TileGrid::Set(int col, int row, GardenTileType type) noexcept
    {
        if (!InBounds(col, row))
        {
            return;
        }

        const size_t index = static_cast<size_t>(row * width_ + col);
        tiles_[index].type = type;
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
