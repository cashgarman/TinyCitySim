#include "Chicken.h"

#include "GardenTile.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace TinyCitySim
{
    namespace
    {
        inline constexpr float kEatGrassThreshold = 0.2f;
        inline constexpr float kSeekGrassThreshold = 0.25f;
        inline constexpr float kEatRatePerSec = 0.25f;
        inline constexpr float kArrivalDistance = 2.0f;
        inline constexpr float kAnimFrameDuration = 0.12f;
        inline constexpr int kWalkFrameCount = 4;
        inline constexpr int kPeckFrameIndex = 4;

        [[nodiscard]] std::uint32_t NextRng(std::uint32_t& state) noexcept
        {
            state = state * 747796405u + 2891336453u;
            const std::uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
            return (word >> 22u) ^ word;
        }

        [[nodiscard]] int NextInt(std::uint32_t& state, int minValue, int maxValue) noexcept
        {
            if (maxValue <= minValue)
            {
                return minValue;
            }

            return minValue + static_cast<int>(NextRng(state) % static_cast<std::uint32_t>(maxValue - minValue));
        }
    }

    int ChickenSimulation::TileColFromPos(const TileGrid& grid, float posX) const noexcept
    {
        const float localX = posX - static_cast<float>(grid.OriginX());
        const int col = static_cast<int>(localX / static_cast<float>(grid.TileSize()));
        return std::clamp(col, 0, grid.Width() - 1);
    }

    int ChickenSimulation::TileRowFromPos(const TileGrid& grid, float posY) const noexcept
    {
        const float localY = posY - static_cast<float>(grid.OriginY());
        const int row = static_cast<int>(localY / static_cast<float>(grid.TileSize()));
        return std::clamp(row, 0, grid.Height() - 1);
    }

    float ChickenSimulation::TileCenterX(const TileGrid& grid, int col) const noexcept
    {
        return static_cast<float>(grid.OriginX() + col * grid.TileSize() + grid.TileSize() / 2);
    }

    float ChickenSimulation::TileCenterY(const TileGrid& grid, int row) const noexcept
    {
        return static_cast<float>(grid.OriginY() + row * grid.TileSize() + grid.TileSize() / 2);
    }

    void ChickenSimulation::Spawn(TileGrid& grid, int count, std::uint32_t seed)
    {
        chickens_.clear();
        std::uint32_t rngState = seed != 0u ? seed : 0xA341316Cu;

        std::vector<TileCoord> spawnTiles;
        for (int row = 0; row < grid.Height(); ++row)
        {
            for (int col = 0; col < grid.Width(); ++col)
            {
                const GardenTile& tile = grid.At(col, row);
                if (tile.type == GardenTileType::Lawn && tile.grassLevel > 0.5f)
                {
                    spawnTiles.push_back(TileCoord{ col, row });
                }
            }
        }

        if (spawnTiles.empty())
        {
            return;
        }

        for (int i = 0; i < count; ++i)
        {
            const TileCoord spawn = spawnTiles[NextInt(rngState, 0, static_cast<int>(spawnTiles.size()))];

            Chicken chicken{};
            chicken.posX = TileCenterX(grid, spawn.col);
            chicken.posY = TileCenterY(grid, spawn.row);
            chicken.targetCol = spawn.col;
            chicken.targetRow = spawn.row;
            chicken.facing = (NextRng(rngState) & 1u) != 0u ? ChickenFacing::Right : ChickenFacing::Left;
            chickens_.push_back(chicken);
        }
    }

    void ChickenSimulation::PickGrassTarget(TileGrid& grid, Chicken& chicken) noexcept
    {
        const int currentCol = TileColFromPos(grid, chicken.posX);
        const int currentRow = TileRowFromPos(grid, chicken.posY);

        int bestCol = -1;
        int bestRow = -1;
        int bestDistance = std::numeric_limits<int>::max();

        for (int row = 0; row < grid.Height(); ++row)
        {
            for (int col = 0; col < grid.Width(); ++col)
            {
                const GardenTile& tile = grid.At(col, row);
                if (tile.type != GardenTileType::Lawn || tile.grassLevel <= kSeekGrassThreshold)
                {
                    continue;
                }

                const int distance = std::abs(col - currentCol) + std::abs(row - currentRow);
                if (distance < bestDistance)
                {
                    bestDistance = distance;
                    bestCol = col;
                    bestRow = row;
                }
            }
        }

        if (bestCol >= 0)
        {
            chicken.targetCol = bestCol;
            chicken.targetRow = bestRow;
            chicken.state = ChickenState::Moving;
            return;
        }

        std::uint32_t rngState = static_cast<std::uint32_t>(currentCol * 73856093u + currentRow * 19349663u);
        PickWanderTarget(grid, chicken, rngState);
    }

    void ChickenSimulation::PickWanderTarget(TileGrid& grid, Chicken& chicken, std::uint32_t& rngState) noexcept
    {
        const int currentCol = TileColFromPos(grid, chicken.posX);
        const int currentRow = TileRowFromPos(grid, chicken.posY);

        std::vector<TileCoord> walkableTiles;
        for (int row = 0; row < grid.Height(); ++row)
        {
            for (int col = 0; col < grid.Width(); ++col)
            {
                if (IsWalkable(grid.At(col, row).type))
                {
                    walkableTiles.push_back(TileCoord{ col, row });
                }
            }
        }

        if (walkableTiles.empty())
        {
            chicken.targetCol = currentCol;
            chicken.targetRow = currentRow;
            chicken.state = ChickenState::Moving;
            return;
        }

        const TileCoord target = walkableTiles[NextInt(rngState, 0, static_cast<int>(walkableTiles.size()))];
        chicken.targetCol = target.col;
        chicken.targetRow = target.row;
        chicken.state = ChickenState::Moving;
    }

    void ChickenSimulation::UpdateAnimation(Chicken& chicken, float dt) noexcept
    {
        chicken.animTimer += dt;
        if (chicken.animTimer >= kAnimFrameDuration)
        {
            chicken.animTimer = 0.0f;
            if (chicken.state == ChickenState::Eating)
            {
                chicken.animFrame = kPeckFrameIndex;
            }
            else
            {
                chicken.animFrame = (chicken.animFrame + 1) % kWalkFrameCount;
            }
        }
    }

    void ChickenSimulation::Update(TileGrid& grid, float dt) noexcept
    {
        if (dt <= 0.0f)
        {
            return;
        }

        std::uint32_t wanderSeed = 0x12345678u;

        for (Chicken& chicken : chickens_)
        {
            const int currentCol = TileColFromPos(grid, chicken.posX);
            const int currentRow = TileRowFromPos(grid, chicken.posY);
            GardenTile& currentTile = grid.MutableAt(currentCol, currentRow);

            if (currentTile.type == GardenTileType::Lawn && currentTile.grassLevel > kEatGrassThreshold)
            {
                chicken.state = ChickenState::Eating;
                chicken.animFrame = kPeckFrameIndex;
                chicken.eatTimer += dt;
                currentTile.grassLevel = std::max(0.0f, currentTile.grassLevel - kEatRatePerSec * dt);

                if (currentTile.grassLevel <= kEatGrassThreshold)
                {
                    PickGrassTarget(grid, chicken);
                }
            }
            else
            {
                chicken.state = ChickenState::Moving;

                const float targetX = TileCenterX(grid, chicken.targetCol);
                const float targetY = TileCenterY(grid, chicken.targetRow);
                const float deltaX = targetX - chicken.posX;
                const float deltaY = targetY - chicken.posY;
                const float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);

                if (distance <= kArrivalDistance)
                {
                    chicken.posX = targetX;
                    chicken.posY = targetY;
                    PickGrassTarget(grid, chicken);
                }
                else
                {
                    const float step = chicken.moveSpeed * dt;
                    const float moveAmount = std::min(step, distance);
                    chicken.posX += (deltaX / distance) * moveAmount;
                    chicken.posY += (deltaY / distance) * moveAmount;

                    if (deltaX > 0.5f)
                    {
                        chicken.facing = ChickenFacing::Right;
                    }
                    else if (deltaX < -0.5f)
                    {
                        chicken.facing = ChickenFacing::Left;
                    }
                }
            }

            UpdateAnimation(chicken, dt);
            wanderSeed += 0x9E3779B9u;
        }
    }
}
