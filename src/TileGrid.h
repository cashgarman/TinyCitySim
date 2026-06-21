#pragma once

#include "GardenTile.h"

#include <optional>
#include <span>
#include <vector>

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

        // Modern C++ (C++20): std::span exposes contiguous tile storage without copying.
        [[nodiscard]] std::span<const GardenTile> Tiles() const noexcept { return tiles_; }

        // Modern C++ (C++11): noexcept accessors — bounds-checked read; cannot throw after construction.
        [[nodiscard]] const GardenTile& At(int col, int row) const noexcept;
        [[nodiscard]] GardenTile& MutableAt(int col, int row) noexcept;
        [[nodiscard]] std::optional<GardenTileType> TryAt(int col, int row) const noexcept;

        void Set(int col, int row, GardenTileType type) noexcept;

    private:
        [[nodiscard]] bool InBounds(int col, int row) const noexcept;

        int width_;
        int height_;
        int tileSize_;
        int originX_ = 0;
        int originY_ = 0;

        // Modern C++ (C++11): std::vector stores a flat row-major 2D grid — index = row * width + col.
        std::vector<GardenTile> tiles_;
    };
}
