#pragma once

#include "TileGrid.h"

#include <cstdint>
#include <random>

namespace TinyCitySim
{
    class GardenGenerator
    {
    public:
        explicit GardenGenerator(std::uint32_t seed = 42u);

        // Fills grid's tile storage; does not change grid dimensions.
        void Generate(TileGrid& grid);

    private:
        // Modern C++ (C++11): std::mt19937 is a seeded, high-quality PRNG — prefer over rand().
        std::mt19937 rng_;

        void Fill(TileGrid& grid, GardenTileType type);
        void PlaceFence(TileGrid& grid);
        void PlacePath(TileGrid& grid);
        void PlaceShed(TileGrid& grid);
        void PlacePond(TileGrid& grid);
        void PlaceFlowerBeds(TileGrid& grid);
        void PlaceVegetablePatch(TileGrid& grid);
        void PlaceTrees(TileGrid& grid);
        void PlacePatio(TileGrid& grid);

        [[nodiscard]] bool CanPaint(const TileGrid& grid, int col, int row) const noexcept;
        [[nodiscard]] int RandomInt(int minInclusive, int maxInclusive);
    };
}
