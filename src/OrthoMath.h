#pragma once

namespace TinyCitySim::Math
{
    struct Float2
    {
        float x = 0.0f;
        float y = 0.0f;
    };

    struct Float4
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float w = 0.0f;
    };

    // Column-major 4x4 matrix for HLSL constant buffers.
    struct Float4x4
    {
        float m[4][4]{};
    };

    // Modern C++ (C++11): constexpr replaces #define for compile-time constants.
    inline constexpr float kPi = 3.14159265358979323846f;

    inline Float4x4 OrthographicProjection(float width, float height) noexcept
    {
        Float4x4 matrix{};

        matrix.m[0][0] = 2.0f / width;
        matrix.m[1][1] = -2.0f / height;
        matrix.m[2][2] = 1.0f;
        matrix.m[3][0] = -1.0f;
        matrix.m[3][1] = 1.0f;
        matrix.m[3][3] = 1.0f;

        return matrix;
    }
}
