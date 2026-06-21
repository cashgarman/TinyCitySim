#include "ProceduralChickenSprite.h"

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
            std::uint8_t a = 0;
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
        };

        void ClearFrame(std::span<std::uint8_t> rgbaOut, int frameIndex)
        {
            for (int y = 0; y < kChickenFrameSize; ++y)
            {
                for (int x = 0; x < kChickenFrameSize; ++x)
                {
                    const size_t index = static_cast<size_t>((y * kChickenAtlasWidth + frameIndex * kChickenFrameSize + x) * 4);
                    rgbaOut[index + 0] = 0;
                    rgbaOut[index + 1] = 0;
                    rgbaOut[index + 2] = 0;
                    rgbaOut[index + 3] = 0;
                }
            }
        }

        void WritePixel(std::span<std::uint8_t> rgbaOut, int frameIndex, int x, int y, Rgba8 color)
        {
            if (x < 0 || y < 0 || x >= kChickenFrameSize || y >= kChickenFrameSize)
            {
                return;
            }

            const size_t index = static_cast<size_t>((y * kChickenAtlasWidth + frameIndex * kChickenFrameSize + x) * 4);
            rgbaOut[index + 0] = color.r;
            rgbaOut[index + 1] = color.g;
            rgbaOut[index + 2] = color.b;
            rgbaOut[index + 3] = color.a;
        }

        void FillEllipse(std::span<std::uint8_t> rgbaOut, int frameIndex, float cx, float cy, float rx, float ry, Rgba8 color)
        {
            const int minX = static_cast<int>(std::floor(cx - rx));
            const int maxX = static_cast<int>(std::ceil(cx + rx));
            const int minY = static_cast<int>(std::floor(cy - ry));
            const int maxY = static_cast<int>(std::ceil(cy + ry));

            for (int y = minY; y <= maxY; ++y)
            {
                for (int x = minX; x <= maxX; ++x)
                {
                    const float dx = (static_cast<float>(x) + 0.5f - cx) / rx;
                    const float dy = (static_cast<float>(y) + 0.5f - cy) / ry;
                    if ((dx * dx + dy * dy) <= 1.0f)
                    {
                        WritePixel(rgbaOut, frameIndex, x, y, color);
                    }
                }
            }
        }

        void FillRect(std::span<std::uint8_t> rgbaOut, int frameIndex, int x0, int y0, int x1, int y1, Rgba8 color)
        {
            for (int y = y0; y <= y1; ++y)
            {
                for (int x = x0; x <= x1; ++x)
                {
                    WritePixel(rgbaOut, frameIndex, x, y, color);
                }
            }
        }

        void DrawChickenFrame(std::span<std::uint8_t> rgbaOut, int frameIndex, float legOffset, float headPeck, Prng& prng)
        {
            ClearFrame(rgbaOut, frameIndex);

            const Rgba8 bodyColor{ 240, 210, 80, 255 };
            const Rgba8 bodyShadow{ 200, 170, 55, 255 };
            const Rgba8 combColor{ 210, 40, 35, 255 };
            const Rgba8 beakColor{ 240, 140, 40, 255 };
            const Rgba8 legColor{ 60, 45, 30, 255 };
            const Rgba8 eyeColor{ 20, 20, 20, 255 };

            FillEllipse(rgbaOut, frameIndex, 8.0f, 9.5f + headPeck * 0.5f, 5.5f, 4.5f, bodyColor);
            FillEllipse(rgbaOut, frameIndex, 7.0f, 10.0f + headPeck * 0.5f, 4.5f, 3.5f, bodyShadow);

            FillRect(rgbaOut, frameIndex, 10, 3, 12, 5, combColor);
            FillRect(rgbaOut, frameIndex, 11, 2, 12, 3, combColor);

            FillEllipse(rgbaOut, frameIndex, 12.5f, 8.5f + headPeck, 2.0f, 1.8f, beakColor);
            WritePixel(rgbaOut, frameIndex, 11, 7, eyeColor);
            WritePixel(rgbaOut, frameIndex, 12, 7, eyeColor);

            const float leftLegX = 6.0f + legOffset;
            const float rightLegX = 10.0f - legOffset;
            FillRect(rgbaOut, frameIndex, static_cast<int>(leftLegX), 12, static_cast<int>(leftLegX), 14, legColor);
            FillRect(rgbaOut, frameIndex, static_cast<int>(rightLegX), 12, static_cast<int>(rightLegX), 14, legColor);

            for (int i = 0; i < 6; ++i)
            {
                const int speckX = 4 + static_cast<int>(prng.NextFloat() * 8.0f);
                const int speckY = 7 + static_cast<int>(prng.NextFloat() * 5.0f);
                WritePixel(rgbaOut, frameIndex, speckX, speckY, bodyShadow);
            }
        }
    }

    void ProceduralChickenSprite::GenerateAtlas(std::uint32_t seed, std::span<std::uint8_t> rgbaOut)
    {
        const size_t expectedSize = static_cast<size_t>(kChickenAtlasWidth * kChickenFrameSize * 4);
        if (rgbaOut.size() < expectedSize)
        {
            return;
        }

        std::memset(rgbaOut.data(), 0, expectedSize);

        Prng prng(seed);

        const float walkOffsets[kChickenFrameCount - 1] =
        {
            -0.5f,
            0.5f,
            -1.0f,
            1.0f,
        };

        for (int frame = 0; frame < kChickenFrameCount - 1; ++frame)
        {
            DrawChickenFrame(rgbaOut, frame, walkOffsets[frame], 0.0f, prng);
        }

        DrawChickenFrame(rgbaOut, kChickenFrameCount - 1, 0.0f, 1.5f, prng);
    }
}
