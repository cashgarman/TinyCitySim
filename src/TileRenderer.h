#pragma once

#include "D3D11Context.h"
#include "OrthoMath.h"
#include "TileGrid.h"
#include "TileTextureAtlas.h"

#include "Platform.h"

#include <Windows.h>
#include <d3d11.h>
#include <cstdint>
#include <optional>
#include <wrl/client.h>

namespace TinyCitySim
{
    class TileRenderer
    {
    public:
        struct Vertex
        {
            Math::Float2 position;
            Math::Float2 uv;
            Math::Float2 tileCoord;
            Math::Float4 color;
            float tileType = 0.0f;
        };

        struct AtlasConstantBuffer
        {
            Math::Float2 atlasGridSize{};
            Math::Float2 atlasInvGridSize{};
            float useSolidColor = 0.0f;
            float cellTexSize = static_cast<float>(kProceduralTileTexSize);
            float elapsedTime = 0.0f;
            float padding = 0.0f;
        };

        TileRenderer() = default;
        ~TileRenderer() = default;

        TileRenderer(const TileRenderer&) = delete;
        TileRenderer& operator=(const TileRenderer&) = delete;

        [[nodiscard]] bool Initialize(ID3D11Device* device, int clientWidth, int clientHeight);
        [[nodiscard]] bool BuildAtlas(const TileGrid& grid, std::uint32_t seed);
        void Resize(int clientWidth, int clientHeight);
        void Draw(const TileGrid& grid, const std::optional<TileCoord>& hoveredTile, float elapsedTime);

    private:
        [[nodiscard]] bool LoadShaders(ID3D11Device* device);
        [[nodiscard]] bool CreateConstantBuffers(ID3D11Device* device);
        void UpdateProjection(int clientWidth, int clientHeight);
        void UpdateAtlasConstants(bool useSolidColor, float elapsedTime);
        void DrawQuad(
            float x,
            float y,
            float width,
            float height,
            const Math::Float2& tileCoord,
            const Math::Float4& color,
            float tileType,
            bool useSolidColor);
        void DrawTileFill(
            const TileGrid& grid,
            int col,
            int row,
            const Math::Float4& color,
            float tileType,
            bool useSolidColor);
        void DrawTileBorder(const TileGrid& grid, int col, int row, float borderWidth, const Math::Float4& color);

        Microsoft::WRL::ComPtr<ID3D11Device> device_;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> projectionConstantBuffer_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> atlasConstantBuffer_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> dynamicVertexBuffer_;

        TileTextureAtlas atlas_;

        int clientWidth_ = 0;
        int clientHeight_ = 0;
        bool atlasReady_ = false;
    };
}
