#pragma once

#include "OrthoMath.h"

#include <string_view>

namespace TinyCitySim
{
    // Modern C++ (C++11): enum class is strongly typed — unlike legacy enum, it does not
    // implicitly convert to int, which prevents accidental mixing of tile types with indices.
    enum class GardenTileType
    {
        Lawn,
        Fence,
        Path,
        FlowerBed,
        VegetablePatch,
        Tree,
        Pond,
        Shed,
        Patio
    };

    // Modern C++ (C++11): Plain struct holds per-cell simulation data; default member
    // initializer means every tile starts as Lawn until the generator overwrites it.
    struct GardenTile
    {
        GardenTileType type = GardenTileType::Lawn;
    };

    // Modern C++ (C++11): [[nodiscard]] warns callers who ignore the return value — useful
    // for lookup helpers where discarding the result is almost certainly a bug.
    [[nodiscard]] inline Math::Float4 ColorFor(GardenTileType type) noexcept
    {
        switch (type)
        {
        case GardenTileType::Lawn:
            return { 0.25f, 0.55f, 0.22f, 1.0f };
        case GardenTileType::Fence:
            return { 0.55f, 0.40f, 0.25f, 1.0f };
        case GardenTileType::Path:
            return { 0.55f, 0.55f, 0.50f, 1.0f };
        case GardenTileType::FlowerBed:
            return { 0.85f, 0.35f, 0.65f, 1.0f };
        case GardenTileType::VegetablePatch:
            return { 0.35f, 0.22f, 0.12f, 1.0f };
        case GardenTileType::Tree:
            return { 0.12f, 0.35f, 0.15f, 1.0f };
        case GardenTileType::Pond:
            return { 0.20f, 0.45f, 0.75f, 1.0f };
        case GardenTileType::Shed:
            return { 0.65f, 0.30f, 0.22f, 1.0f };
        case GardenTileType::Patio:
            return { 0.75f, 0.73f, 0.68f, 1.0f };
        }

        return { 0.25f, 0.55f, 0.22f, 1.0f };
    }

    // Modern C++ (C++17): std::wstring_view is a non-owning reference to wide text — no
    // heap allocation when passing display names to the log panel (see ModernCppGuide.h).
    [[nodiscard]] inline std::wstring_view NameFor(GardenTileType type) noexcept
    {
        switch (type)
        {
        case GardenTileType::Lawn:
            return L"Lawn";
        case GardenTileType::Fence:
            return L"Fence";
        case GardenTileType::Path:
            return L"Path";
        case GardenTileType::FlowerBed:
            return L"FlowerBed";
        case GardenTileType::VegetablePatch:
            return L"VegetablePatch";
        case GardenTileType::Tree:
            return L"Tree";
        case GardenTileType::Pond:
            return L"Pond";
        case GardenTileType::Shed:
            return L"Shed";
        case GardenTileType::Patio:
            return L"Patio";
        }

        return L"Unknown";
    }
}
