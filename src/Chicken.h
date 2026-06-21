#pragma once

#include "TileGrid.h"
#include "Walkability.h"

#include <cstdint>
#include <vector>

namespace TinyCitySim
{
    enum class ChickenFacing
    {
        Left,
        Right
    };

    enum class ChickenState
    {
        Moving,
        Eating
    };

    struct Chicken
    {
        float posX = 0.0f;
        float posY = 0.0f;
        int targetCol = 0;
        int targetRow = 0;
        float moveSpeed = 48.0f;
        float eatTimer = 0.0f;
        float animTimer = 0.0f;
        int animFrame = 0;
        ChickenFacing facing = ChickenFacing::Right;
        ChickenState state = ChickenState::Moving;
    };

    class ChickenSimulation
    {
    public:
        void Spawn(TileGrid& grid, int count, std::uint32_t seed);
        void Update(TileGrid& grid, float dt) noexcept;

        [[nodiscard]] const std::vector<Chicken>& Chickens() const noexcept { return chickens_; }

    private:
        [[nodiscard]] int TileColFromPos(const TileGrid& grid, float posX) const noexcept;
        [[nodiscard]] int TileRowFromPos(const TileGrid& grid, float posY) const noexcept;
        [[nodiscard]] float TileCenterX(const TileGrid& grid, int col) const noexcept;
        [[nodiscard]] float TileCenterY(const TileGrid& grid, int row) const noexcept;
        void PickGrassTarget(TileGrid& grid, Chicken& chicken) noexcept;
        void PickWanderTarget(TileGrid& grid, Chicken& chicken, std::uint32_t& rngState) noexcept;
        void UpdateAnimation(Chicken& chicken, float dt) noexcept;

        std::vector<Chicken> chickens_;
    };
}
