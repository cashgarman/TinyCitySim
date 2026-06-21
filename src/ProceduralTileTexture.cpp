#include "ProceduralTileTexture.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace TinyCitySim
{
    namespace
    {
        struct Rgba8
        {
            std::uint8_t r = 0;
            std::uint8_t g = 0;
            std::uint8_t b = 0;
            std::uint8_t a = 255;
        };

        struct Prng
        {
            std::uint32_t state = 0;

            explicit Prng(std::uint32_t seed) noexcept
                : state(seed != 0u ? seed : 0xA341316Cu)
            {
            }

            [[nodiscard]] std::uint32_t Next() noexcept
            {
                state = state * 747796405u + 2891336453u;
                const std::uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
                return (word >> 22u) ^ word;
            }

            [[nodiscard]] float NextFloat() noexcept
            {
                return static_cast<float>(Next() & 0xFFFFFFu) / static_cast<float>(0x1000000u);
            }

            [[nodiscard]] int NextInt(int minValue, int maxValue) noexcept
            {
                if (maxValue <= minValue)
                {
                    return minValue;
                }

                return minValue + static_cast<int>(Next() % static_cast<std::uint32_t>(maxValue - minValue));
            }
        };

        // The function SeedForCell generates a pseudo-random unique seed for a grid cell, given its type, column, row, and a global seed.
        [[nodiscard]] std::uint32_t SeedForCell(
            GardenTileType type, // Receives the type of the garden tile to allow type-specific variation in noise/texture/randomization.
            int col,             // The column index of the tile, ensures seed is unique per horizontal cell.
            int row,             // The row index of the tile, ensures seed is unique per vertical cell.
            std::uint32_t globalSeed) noexcept // The globalSeed ensures that garden-wide randomization is reproducible (changing this changes the whole garden).
        {
            // Convert the enum type to a uint32_t to use in bitwise operations.
            const std::uint32_t typeBits = static_cast<std::uint32_t>(type);
            // Combines inputs with unique large primes and XOR, which helps decorrelate dimensions; ensures distinct seeds for (type, col, row, globalSeed) tuples.
            return globalSeed
                ^ (static_cast<std::uint32_t>(col) * 73856093u)   // col multiplied by a large prime to distribute columns uniquely.
                ^ (static_cast<std::uint32_t>(row) * 19349663u)   // row multiplied by a different large prime for unique row impact.
                ^ (typeBits * 83492791u);                         // typeBits multiplied by another large prime for type uniqueness.
        }

        [[nodiscard]] Rgba8 FromFloat4(const Math::Float4& color) noexcept
        {
            return Rgba8
            {
                static_cast<std::uint8_t>(std::clamp(color.x * 255.0f, 0.0f, 255.0f)),
                static_cast<std::uint8_t>(std::clamp(color.y * 255.0f, 0.0f, 255.0f)),
                static_cast<std::uint8_t>(std::clamp(color.z * 255.0f, 0.0f, 255.0f)),
                static_cast<std::uint8_t>(std::clamp(color.w * 255.0f, 0.0f, 255.0f)),
            };
        }

        [[nodiscard]] Rgba8 LerpColor(const Rgba8& a, const Rgba8& b, float t) noexcept
        {
            const float clamped = std::clamp(t, 0.0f, 1.0f);
            return Rgba8
            {
                static_cast<std::uint8_t>(a.r + (b.r - a.r) * clamped),
                static_cast<std::uint8_t>(a.g + (b.g - a.g) * clamped),
                static_cast<std::uint8_t>(a.b + (b.b - a.b) * clamped),
                255,
            };
        }

        [[nodiscard]] Rgba8 VaryColor(const Rgba8& base, float amount, Prng& prng) noexcept
        {
            const float delta = (prng.NextFloat() * 2.0f - 1.0f) * amount;
            return Rgba8
            {
                static_cast<std::uint8_t>(std::clamp(base.r + delta * 255.0f, 0.0f, 255.0f)),
                static_cast<std::uint8_t>(std::clamp(base.g + delta * 255.0f, 0.0f, 255.0f)),
                static_cast<std::uint8_t>(std::clamp(base.b + delta * 255.0f, 0.0f, 255.0f)),
                255,
            };
        }

        void WritePixel(std::span<std::uint8_t> rgbaOut, int size, int x, int y, Rgba8 color)
        {
            if (x < 0 || y < 0 || x >= size || y >= size)
            {
                return;
            }

            const size_t index = static_cast<size_t>((y * size + x) * 4);
            rgbaOut[index + 0] = color.r;
            rgbaOut[index + 1] = color.g;
            rgbaOut[index + 2] = color.b;
            rgbaOut[index + 3] = color.a;
        }

        [[nodiscard]] float HashNoise(int x, int y, std::uint32_t seed) noexcept
        {
            std::uint32_t hash = static_cast<std::uint32_t>(x) * 374761393u
                + static_cast<std::uint32_t>(y) * 668265263u
                + seed * 362437u;
            hash = (hash ^ (hash >> 13u)) * 1274126177u;
            return static_cast<float>(hash & 0xFFFFu) / static_cast<float>(0xFFFFu);
        }

        void GenerateLawn(int size, std::span<std::uint8_t> rgbaOut, Prng& prng)
        {
            const Rgba8 base = FromFloat4(ColorFor(GardenTileType::Lawn));
            const Rgba8 dark = LerpColor(base, Rgba8{ 0, 0, 0, 255 }, 0.25f);
            const Rgba8 light = LerpColor(base, Rgba8{ 255, 255, 255, 255 }, 0.12f);

            for (int y = 0; y < size; ++y)
            {
                for (int x = 0; x < size; ++x)
                {
                    const float noise = HashNoise(x, y, prng.Next());
                    Rgba8 color = LerpColor(dark, light, noise);

                    if (prng.NextFloat() < 0.08f)
                    {
                        color = VaryColor(dark, 0.15f, prng);
                    }

                    WritePixel(rgbaOut, size, x, y, color);
                }
            }
        }

        void GenerateFence(const TileGenContext& ctx, std::span<std::uint8_t> rgbaOut)
        {
            const int size = ctx.size;
            const Rgba8 base = FromFloat4(ColorFor(GardenTileType::Fence));
            const Rgba8 dark = LerpColor(base, Rgba8{ 0, 0, 0, 255 }, 0.18f);
            const Rgba8 light = LerpColor(base, Rgba8{ 255, 255, 255, 255 }, 0.10f);
            const int plankWidth = std::max(3, size / 6);
            const std::uint32_t structSeed = SeedForCell(GardenTileType::Fence, ctx.col, ctx.row, ctx.seed);

            const bool neighborNorth = ctx.north == GardenTileType::Fence;
            const bool neighborSouth = ctx.south == GardenTileType::Fence;
            const bool neighborEast = ctx.east == GardenTileType::Fence;
            const bool neighborWest = ctx.west == GardenTileType::Fence;

            const bool verticalRun = neighborEast && neighborWest;
            const bool horizontalRun = neighborNorth && neighborSouth;
            const int postSize = std::max(4, size / 6);

            for (int y = 0; y < size; ++y)
            {
                for (int x = 0; x < size; ++x)
                {
                    const int worldX = ctx.col * size + x;
                    const int worldY = ctx.row * size + y;

                    Rgba8 color = dark;

                    if (horizontalRun && !verticalRun)
                    {
                        const int plankIndex = worldY / plankWidth;
                        const bool seam = (worldY % plankWidth) == 0;
                        const float grain = HashNoise(plankIndex, worldX, structSeed);
                        color = seam ? dark : LerpColor(dark, light, grain * 0.6f + 0.2f);
                    }
                    else
                    {
                        const int plankIndex = worldX / plankWidth;
                        const bool seam = (worldX % plankWidth) == 0;
                        const float grain = HashNoise(plankIndex, worldY, structSeed);
                        color = seam ? dark : LerpColor(dark, light, grain * 0.6f + 0.2f);
                    }

                    if (y < 2 && !neighborNorth)
                    {
                        color = dark;
                    }

                    if (y >= size - 2 && !neighborSouth)
                    {
                        color = dark;
                    }

                    if (!neighborWest && x < 2)
                    {
                        color = dark;
                    }

                    if (!neighborEast && x >= size - 2)
                    {
                        color = dark;
                    }

                    const bool nwCorner = !neighborNorth && !neighborWest && x < postSize && y < postSize;
                    const bool neCorner = !neighborNorth && !neighborEast && x >= size - postSize && y < postSize;
                    const bool swCorner = !neighborSouth && !neighborWest && x < postSize && y >= size - postSize;
                    const bool seCorner = !neighborSouth && !neighborEast && x >= size - postSize && y >= size - postSize;

                    if (nwCorner || neCorner || swCorner || seCorner)
                    {
                        color = LerpColor(dark, Rgba8{ 0, 0, 0, 255 }, 0.25f);
                    }

                    WritePixel(rgbaOut, size, x, y, color);
                }
            }
        }

        void GeneratePath(int size, std::span<std::uint8_t> rgbaOut, Prng& prng)
        {
            const Rgba8 stone = FromFloat4(ColorFor(GardenTileType::Path));
            const Rgba8 mortar = LerpColor(stone, Rgba8{ 255, 255, 255, 255 }, 0.18f);
            const int cellSize = std::max(4, size / 5);

            for (int y = 0; y < size; ++y)
            {
                for (int x = 0; x < size; ++x)
                {
                    const int cellX = x / cellSize;
                    const int cellY = y / cellSize;
                    const bool border = (x % cellSize == 0) || (y % cellSize == 0);

                    if (border)
                    {
                        WritePixel(rgbaOut, size, x, y, mortar);
                    }
                    else
                    {
                        const float shade = HashNoise(cellX, cellY, prng.Next());
                        WritePixel(rgbaOut, size, x, y, LerpColor(stone, mortar, shade * 0.25f));
                    }
                }
            }
        }

        void GenerateFlowerBed(int size, std::span<std::uint8_t> rgbaOut, Prng& prng)
        {
            const Rgba8 soil = LerpColor(FromFloat4(ColorFor(GardenTileType::FlowerBed)), Rgba8{ 0, 0, 0, 255 }, 0.45f);
            const Rgba8 flowerColors[] =
            {
                { 230, 60, 120, 255 },
                { 255, 210, 60, 255 },
                { 180, 80, 220, 255 },
                { 255, 120, 80, 255 },
            };

            for (int y = 0; y < size; ++y)
            {
                for (int x = 0; x < size; ++x)
                {
                    const float noise = HashNoise(x, y, prng.Next());
                    WritePixel(rgbaOut, size, x, y, LerpColor(soil, Rgba8{ 255, 255, 255, 255 }, noise * 0.08f));
                }
            }

            const int flowerCount = prng.NextInt(5, 10);
            for (int i = 0; i < flowerCount; ++i)
            {
                const int cx = prng.NextInt(2, size - 2);
                const int cy = prng.NextInt(2, size - 2);
                const Rgba8 petal = flowerColors[prng.NextInt(0, static_cast<int>(std::size(flowerColors)))];

                for (int dy = -1; dy <= 1; ++dy)
                {
                    for (int dx = -1; dx <= 1; ++dx)
                    {
                        if (dx == 0 && dy == 0)
                        {
                            WritePixel(rgbaOut, size, cx, cy, Rgba8{ 255, 230, 80, 255 });
                        }
                        else
                        {
                            WritePixel(rgbaOut, size, cx + dx, cy + dy, petal);
                        }
                    }
                }
            }
        }

        void GenerateVegetablePatch(int size, std::span<std::uint8_t> rgbaOut, Prng& prng)
        {
            const Rgba8 soil = FromFloat4(ColorFor(GardenTileType::VegetablePatch));
            const Rgba8 crop = LerpColor(soil, Rgba8{ 40, 160, 50, 255 }, 0.65f);
            const int rowHeight = std::max(3, size / 6);

            for (int y = 0; y < size; ++y)
            {
                for (int x = 0; x < size; ++x)
                {
                    const int row = y / rowHeight;
                    const bool isCropRow = (row % 2) == 0;
                    const float shade = HashNoise(x, row, prng.Next());

                    if (isCropRow)
                    {
                        Rgba8 color = LerpColor(crop, Rgba8{ 255, 255, 255, 255 }, shade * 0.15f);
                        if (x % 4 == 0)
                        {
                            color = LerpColor(color, Rgba8{ 30, 100, 35, 255 }, 0.35f);
                        }
                        WritePixel(rgbaOut, size, x, y, color);
                    }
                    else
                    {
                        WritePixel(rgbaOut, size, x, y, LerpColor(soil, Rgba8{ 255, 255, 255, 255 }, shade * 0.06f));
                    }
                }
            }
        }

        void GenerateTree(int size, std::span<std::uint8_t> rgbaOut, Prng& prng)
        {
            const Rgba8 trunk = LerpColor(FromFloat4(ColorFor(GardenTileType::Tree)), Rgba8{ 80, 50, 20, 255 }, 0.55f);
            const Rgba8 canopy = FromFloat4(ColorFor(GardenTileType::Tree));
            const Rgba8 canopyDark = LerpColor(canopy, Rgba8{ 0, 0, 0, 255 }, 0.20f);
            const float centerX = static_cast<float>(size) * 0.5f;
            const float centerY = static_cast<float>(size) * 0.38f;
            const float radius = static_cast<float>(size) * 0.38f;
            const int trunkTop = size * 2 / 3;

            for (int y = 0; y < size; ++y)
            {
                for (int x = 0; x < size; ++x)
                {
                    if (y >= trunkTop && std::abs(x - size / 2) <= 2)
                    {
                        WritePixel(rgbaOut, size, x, y, trunk);
                        continue;
                    }

                    const float dx = static_cast<float>(x) + 0.5f - centerX;
                    const float dy = static_cast<float>(y) + 0.5f - centerY;
                    const float dist = std::sqrt(dx * dx + dy * dy);

                    if (dist <= radius)
                    {
                        const float shade = HashNoise(x, y, prng.Next());
                        const Rgba8 leaf = LerpColor(canopyDark, canopy, shade);
                        WritePixel(rgbaOut, size, x, y, leaf);
                    }
                    else
                    {
                        WritePixel(rgbaOut, size, x, y, LerpColor(trunk, Rgba8{ 60, 120, 40, 255 }, 0.35f));
                    }
                }
            }
        }

        void GeneratePond(int size, std::span<std::uint8_t> rgbaOut, Prng& prng)
        {
            const Rgba8 deep = FromFloat4(ColorFor(GardenTileType::Pond));
            const Rgba8 shallow = LerpColor(deep, Rgba8{ 255, 255, 255, 255 }, 0.25f);
            const float centerX = static_cast<float>(size) * 0.5f;
            const float centerY = static_cast<float>(size) * 0.5f;

            for (int y = 0; y < size; ++y)
            {
                for (int x = 0; x < size; ++x)
                {
                    const float dx = (static_cast<float>(x) - centerX) / static_cast<float>(size);
                    const float dy = (static_cast<float>(y) - centerY) / static_cast<float>(size);
                    const float dist = std::sqrt(dx * dx + dy * dy);
                    const float ripple = std::sin((dist * 18.0f + prng.NextFloat() * 2.0f) * Math::kPi) * 0.5f + 0.5f;
                    const Rgba8 color = LerpColor(deep, shallow, ripple * (1.0f - dist * 1.2f));
                    WritePixel(rgbaOut, size, x, y, color);
                }
            }
        }

        void GenerateShed(const TileGenContext& ctx, std::span<std::uint8_t> rgbaOut)
        {
            const int size = ctx.size;
            const Rgba8 wall = FromFloat4(ColorFor(GardenTileType::Shed));
            const Rgba8 roof = LerpColor(wall, Rgba8{ 0, 0, 0, 255 }, 0.35f);
            const Rgba8 plankDark = LerpColor(wall, Rgba8{ 0, 0, 0, 255 }, 0.12f);
            const int roofHeight = std::max(4, size * 3 / 10);
            const std::uint32_t structSeed = SeedForCell(GardenTileType::Shed, ctx.col, ctx.row, ctx.seed);

            const bool neighborNorth = ctx.north == GardenTileType::Shed;
            const bool neighborSouth = ctx.south == GardenTileType::Shed;
            const bool neighborEast = ctx.east == GardenTileType::Shed;
            const bool neighborWest = ctx.west == GardenTileType::Shed;

            const bool isTopRow = !neighborNorth;
            const bool isDoorCell = neighborEast && neighborSouth && !neighborWest && !neighborNorth;

            for (int y = 0; y < size; ++y)
            {
                for (int x = 0; x < size; ++x)
                {
                    const int worldY = ctx.row * size + y;

                    if (isTopRow && y < roofHeight)
                    {
                        Rgba8 color = roof;
                        if (y == 0)
                        {
                            color = LerpColor(roof, Rgba8{ 0, 0, 0, 255 }, 0.15f);
                        }

                        WritePixel(rgbaOut, size, x, y, color);
                        continue;
                    }

                    const bool seam = (worldY % 4) == 0;
                    const float grain = HashNoise(ctx.col * size + x, worldY, structSeed);
                    Rgba8 color = LerpColor(wall, plankDark, grain * 0.25f);

                    if (seam)
                    {
                        color = plankDark;
                    }

                    if (isDoorCell
                        && x >= size / 2 - 1
                        && x <= size - 3
                        && y >= size - 10
                        && y <= size - 3)
                    {
                        color = LerpColor(plankDark, Rgba8{ 0, 0, 0, 255 }, 0.35f);
                    }

                    WritePixel(rgbaOut, size, x, y, color);
                }
            }
        }

        void GeneratePatio(int size, std::span<std::uint8_t> rgbaOut, Prng& prng)
        {
            const Rgba8 stone = FromFloat4(ColorFor(GardenTileType::Patio));
            const Rgba8 mortar = LerpColor(stone, Rgba8{ 0, 0, 0, 255 }, 0.12f);
            const int brickW = std::max(5, size / 4);
            const int brickH = std::max(3, size / 8);

            for (int y = 0; y < size; ++y)
            {
                for (int x = 0; x < size; ++x)
                {
                    const int row = y / brickH;
                    const int offset = (row % 2) * (brickW / 2);
                    const int localX = (x + size - offset % size) % size;
                    const bool seam = (localX % brickW == 0) || (y % brickH == 0);

                    if (seam)
                    {
                        WritePixel(rgbaOut, size, x, y, mortar);
                    }
                    else
                    {
                        const float shade = HashNoise(x / brickW, y / brickH, prng.Next());
                        WritePixel(rgbaOut, size, x, y, LerpColor(stone, mortar, shade * 0.18f));
                    }
                }
            }
        }
    }

    void ProceduralTileTexture::GenerateCell(const TileGenContext& ctx, std::span<std::uint8_t> rgbaOut)
    {
        const size_t expectedBytes = static_cast<size_t>(ctx.size * ctx.size * 4);
        if (rgbaOut.size() < expectedBytes)
        {
            return;
        }

        std::memset(rgbaOut.data(), 0, rgbaOut.size());

        Prng prng(SeedForCell(ctx.type, ctx.col, ctx.row, ctx.seed));

        switch (ctx.type)
        {
        case GardenTileType::Lawn:
            GenerateLawn(ctx.size, rgbaOut, prng);
            break;
        case GardenTileType::Fence:
            GenerateFence(ctx, rgbaOut);
            break;
        case GardenTileType::Path:
            GeneratePath(ctx.size, rgbaOut, prng);
            break;
        case GardenTileType::FlowerBed:
            GenerateFlowerBed(ctx.size, rgbaOut, prng);
            break;
        case GardenTileType::VegetablePatch:
            GenerateVegetablePatch(ctx.size, rgbaOut, prng);
            break;
        case GardenTileType::Tree:
            GenerateTree(ctx.size, rgbaOut, prng);
            break;
        case GardenTileType::Pond:
            GeneratePond(ctx.size, rgbaOut, prng);
            break;
        case GardenTileType::Shed:
            GenerateShed(ctx, rgbaOut);
            break;
        case GardenTileType::Patio:
            GeneratePatio(ctx.size, rgbaOut, prng);
            break;
        }
    }
}
