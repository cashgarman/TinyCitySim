#include "GardenGenerator.h"

#include <cmath>

namespace TinyCitySim
{
    GardenGenerator::GardenGenerator(std::uint32_t seed)
        : rng_(seed)
    {
        // Modern C++ (C++11): explicit constructor prevents accidental GardenGenerator g = 42;
        // the seed makes layouts reproducible — change it to explore different gardens.
    }

    void GardenGenerator::Generate(TileGrid& grid)
    {
        // Phase ordering keeps each step readable — a monolithic nested loop would be harder
        // to debug. Each phase only overwrites Lawn tiles via CanPaint(), so earlier features
        // (fence, path, shed) are never accidentally clobbered by later ones.
        Fill(grid, GardenTileType::Lawn);
        PlaceFence(grid);
        PlacePath(grid);
        PlaceShed(grid);
        PlacePond(grid);
        PlaceFlowerBeds(grid);
        PlaceVegetablePatch(grid);
        PlaceTrees(grid);
        PlacePatio(grid);
    }

    void GardenGenerator::Fill(TileGrid& grid, GardenTileType type)
    {
        // Row-major iteration: outer loop is row (y), inner loop is col (x) — matches how
        // TileGrid stores tiles_ as a flat vector with index = row * width + col.
        for (int row = 0; row < grid.Height(); ++row)
        {
            for (int col = 0; col < grid.Width(); ++col)
            {
                grid.Set(col, row, type);
            }
        }
    }

    void GardenGenerator::PlaceFence(TileGrid& grid)
    {
        const int width = grid.Width();
        const int height = grid.Height();

        // Gate gap at bottom-center lets the path enter from outside the yard.
        const int gateCol = width / 2;
        const int gateRow = height - 1;

        for (int row = 0; row < height; ++row)
        {
            for (int col = 0; col < width; ++col)
            {
                const bool onPerimeter = col == 0 || col == width - 1 || row == 0 || row == height - 1;
                const bool isGate = col == gateCol && row == gateRow;

                if (onPerimeter && !isGate)
                {
                    grid.Set(col, row, GardenTileType::Fence);
                }
            }
        }
    }

    void GardenGenerator::PlacePath(TileGrid& grid)
    {
        const int width = grid.Width();
        const int height = grid.Height();
        const int gateCol = width / 2;
        const int gateRow = height - 1;

        // L-shaped path: vertical segment from the gate inward, then horizontal toward top-right.
        // turnRow divides the grid into a lower approach and an upper garden area.
        const int turnRow = height / 3;

        grid.Set(gateCol, gateRow, GardenTileType::Path);

        for (int row = gateRow - 1; row >= turnRow; --row)
        {
            grid.Set(gateCol, row, GardenTileType::Path);
        }

        for (int col = gateCol + 1; col <= width - 3; ++col)
        {
            grid.Set(col, turnRow, GardenTileType::Path);
        }
    }

    void GardenGenerator::PlaceShed(TileGrid& grid)
    {
        const int width = grid.Width();
        const int shedCol = width - 3;
        const int shedRow = 1;

        for (int dy = 0; dy < 2; ++dy)
        {
            for (int dx = 0; dx < 2; ++dx)
            {
                const int col = shedCol + dx;
                const int row = shedRow + dy;

                if (CanPaint(grid, col, row))
                {
                    grid.Set(col, row, GardenTileType::Shed);
                }
            }
        }
    }

    void GardenGenerator::PlacePond(TileGrid& grid)
    {
        const int width = grid.Width();
        const int height = grid.Height();

        // Pick a random interior anchor; skip if the blob would overlap path or shed.
        const int anchorCol = RandomInt(3, width - 5);
        const int anchorRow = RandomInt(4, height - 5);
        // Manhattan-radius flood: |dx| + |dy| <= radius produces a diamond-shaped blob (~5 cells
        // at radius 2). Skip painting if overlap with path/shed left too few cells.
        const int radius = 2;

        int painted = 0;

        for (int row = anchorRow - radius; row <= anchorRow + radius; ++row)
        {
            for (int col = anchorCol - radius; col <= anchorCol + radius; ++col)
            {
                const int manhattan = std::abs(col - anchorCol) + std::abs(row - anchorRow);

                if (manhattan <= radius && CanPaint(grid, col, row))
                {
                    grid.Set(col, row, GardenTileType::Pond);
                    ++painted;
                }
            }
        }

        // If overlap prevented a readable pond, try a fallback spot in the lower-center lawn.
        if (painted < 3)
        {
            const int fallbackCol = width / 2 - 2;
            const int fallbackRow = height / 2;

            for (int row = fallbackRow - 1; row <= fallbackRow + 1; ++row)
            {
                for (int col = fallbackCol - 1; col <= fallbackCol + 1; ++col)
                {
                    if (CanPaint(grid, col, row))
                    {
                        grid.Set(col, row, GardenTileType::Pond);
                    }
                }
            }
        }
    }

    void GardenGenerator::PlaceFlowerBeds(TileGrid& grid)
    {
        const int bedCount = RandomInt(2, 4);

        for (int bed = 0; bed < bedCount; ++bed)
        {
            const int bedWidth = RandomInt(2, 3);
            const int bedHeight = 2;
            const int col = RandomInt(2, grid.Width() - bedWidth - 2);
            const int row = RandomInt(2, grid.Height() - bedHeight - 2);

            bool fits = true;

            for (int dy = 0; dy < bedHeight && fits; ++dy)
            {
                for (int dx = 0; dx < bedWidth; ++dx)
                {
                    if (!CanPaint(grid, col + dx, row + dy))
                    {
                        fits = false;
                        break;
                    }
                }
            }

            if (!fits)
            {
                continue;
            }

            for (int dy = 0; dy < bedHeight; ++dy)
            {
                for (int dx = 0; dx < bedWidth; ++dx)
                {
                    grid.Set(col + dx, row + dy, GardenTileType::FlowerBed);
                }
            }
        }
    }

    void GardenGenerator::PlaceVegetablePatch(TileGrid& grid)
    {
        const int patchWidth = 4;
        const int patchHeight = 3;
        const int col = 1;
        const int row = grid.Height() - patchHeight - 1;

        for (int dy = 0; dy < patchHeight; ++dy)
        {
            for (int dx = 0; dx < patchWidth; ++dx)
            {
                if (CanPaint(grid, col + dx, row + dy))
                {
                    grid.Set(col + dx, row + dy, GardenTileType::VegetablePatch);
                }
            }
        }
    }

    void GardenGenerator::PlaceTrees(TileGrid& grid)
    {
        const int treeCount = RandomInt(3, 5);
        int placed = 0;
        int attempts = 0;
        const int maxAttempts = 100;

        while (placed < treeCount && attempts < maxAttempts)
        {
            ++attempts;

            const int col = RandomInt(1, grid.Width() - 2);
            const int row = RandomInt(1, grid.Height() - 2);

            if (CanPaint(grid, col, row))
            {
                grid.Set(col, row, GardenTileType::Tree);
                ++placed;
            }
        }
    }

    void GardenGenerator::PlacePatio(TileGrid& grid)
    {
        const int width = grid.Width();
        const int shedCol = width - 3;
        const int patioCol = shedCol - 1;
        const int patioRow = 3;
        const int patioWidth = 3;
        const int patioHeight = 2;

        for (int dy = 0; dy < patioHeight; ++dy)
        {
            for (int dx = 0; dx < patioWidth; ++dx)
            {
                const int col = patioCol + dx;
                const int row = patioRow + dy;

                if (CanPaint(grid, col, row))
                {
                    grid.Set(col, row, GardenTileType::Patio);
                }
            }
        }
    }

    bool GardenGenerator::CanPaint(const TileGrid& grid, int col, int row) const noexcept
    {
        if (col < 0 || col >= grid.Width() || row < 0 || row >= grid.Height())
        {
            return false;
        }

        // Only overwrite open lawn — fences, paths, and structures stay put.
        return grid.At(col, row).type == GardenTileType::Lawn;
    }

    int GardenGenerator::RandomInt(int minInclusive, int maxInclusive)
    {
        // Modern C++ (C++11): std::uniform_int_distribution + std::mt19937 replaces rand() % n,
        // which has modulo bias and poor statistical quality — bad for reproducible level gen.
        std::uniform_int_distribution<int> dist(minInclusive, maxInclusive);
        return dist(rng_);
    }
}
