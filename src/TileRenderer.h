#pragma once

#include "D3D11Context.h"
#include "OrthoMath.h"
#include "TileGrid.h"

#include "Platform.h"

#include <Windows.h>
#include <d3d11.h>
#include <span>
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
            Math::Float4 color;
        };

        TileRenderer() = default;
        ~TileRenderer() = default;

        TileRenderer(const TileRenderer&) = delete;
        TileRenderer& operator=(const TileRenderer&) = delete;

        [[nodiscard]] bool Initialize(ID3D11Device* device, int clientWidth, int clientHeight);
        void Resize(int clientWidth, int clientHeight);
        void Draw(const TileGrid& grid, const std::optional<TileCoord>& hoveredTile);

    private:
        [[nodiscard]] bool LoadShaders(ID3D11Device* device);
        [[nodiscard]] bool CreateConstantBuffer(ID3D11Device* device);
        void UpdateProjection(int clientWidth, int clientHeight);
        void DrawQuad(float x, float y, float width, float height, const Math::Float4& color);
        void DrawTileFill(const TileGrid& grid, int col, int row, const Math::Float4& color);
        void DrawTileBorder(const TileGrid& grid, int col, int row, float borderWidth, const Math::Float4& color);

        Microsoft::WRL::ComPtr<ID3D11Device> device_;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> dynamicVertexBuffer_;

        int clientWidth_ = 0;
        int clientHeight_ = 0;
    };
}
