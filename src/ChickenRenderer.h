#pragma once

#include "Chicken.h"
#include "ChickenSpriteAtlas.h"
#include "OrthoMath.h"
#include "TileGrid.h"

#include "Platform.h"

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

namespace TinyCitySim
{
    class ChickenRenderer
    {
    public:
        struct Vertex
        {
            Math::Float2 position;
            Math::Float2 uv;
            float flipX = 1.0f;
        };

        struct SpriteConstantBuffer
        {
            float frameCount = static_cast<float>(kChickenFrameCount);
            float frameWidth = static_cast<float>(kChickenFrameSize);
            float atlasWidth = static_cast<float>(kChickenAtlasWidth);
            float padding = 0.0f;
        };

        ChickenRenderer() = default;
        ~ChickenRenderer() = default;

        ChickenRenderer(const ChickenRenderer&) = delete;
        ChickenRenderer& operator=(const ChickenRenderer&) = delete;

        [[nodiscard]] bool Initialize(ID3D11Device* device, int clientWidth, int clientHeight);
        [[nodiscard]] bool BuildAtlas(std::uint32_t seed);
        void Resize(int clientWidth, int clientHeight);
        void Draw(const ChickenSimulation& simulation, const TileGrid& grid, float elapsedTime);

    private:
        [[nodiscard]] bool LoadShaders(ID3D11Device* device);
        [[nodiscard]] bool CreateConstantBuffers(ID3D11Device* device);
        void UpdateProjection(int clientWidth, int clientHeight);
        void UpdateSpriteConstants();
        void DrawSprite(float centerX, float centerY, float size, int frameIndex, bool faceLeft);

        Microsoft::WRL::ComPtr<ID3D11Device> device_;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> projectionConstantBuffer_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> spriteConstantBuffer_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> dynamicVertexBuffer_;
        Microsoft::WRL::ComPtr<ID3D11BlendState> alphaBlendState_;

        ChickenSpriteAtlas atlas_;

        int clientWidth_ = 0;
        int clientHeight_ = 0;
        bool atlasReady_ = false;
    };
}
