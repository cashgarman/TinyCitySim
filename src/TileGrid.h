#pragma once

#include <optional>

namespace TinyCitySim
{
    // Modern C++ (C++11): Plain struct with operator== instead of separate col/row out-params.
    struct TileCoord
    {
        int col = 0;
        int row = 0;

        bool operator==(const TileCoord& other) const noexcept
        {
            return col == other.col && row == other.row;
        }

        bool operator!=(const TileCoord& other) const noexcept
        {
            return !(*this == other);
        }
    };

    // Modern C++ (C++11): constexpr replaces #define TILE_SIZE / GRID_WIDTH macros.
    inline constexpr int kDefaultGridWidth = 20;
    inline constexpr int kDefaultGridHeight = 15;
    inline constexpr int kDefaultTileSize = 32;

    class TileGrid
    {
    public:
        TileGrid(int width, int height, int tileSize);

        void SetClientSize(int clientWidth, int clientHeight) noexcept;

        // Modern C++ (C++17): std::optional replaces sentinel (-1,-1) for miss.
        [[nodiscard]] std::optional<TileCoord> ScreenToTile(int screenX, int screenY) const noexcept;

        [[nodiscard]] int Width() const noexcept { return width_; }
        [[nodiscard]] int Height() const noexcept { return height_; }
        [[nodiscard]] int TileSize() const noexcept { return tileSize_; }
        [[nodiscard]] int OriginX() const noexcept { return originX_; }
        [[nodiscard]] int OriginY() const noexcept { return originY_; }

    private:
        int width_;
        int height_;
        int tileSize_;
        int originX_ = 0;
        int originY_ = 0;
    };
}
