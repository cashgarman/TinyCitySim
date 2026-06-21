#pragma once

#include "ProceduralTileTexture.h"
#include "TileGrid.h"

#include "Platform.h"

#include <d3d11.h>
#include <cstdint>
#include <wrl/client.h>

namespace TinyCitySim
{
    class TileTextureAtlas
    {
    public:
        [[nodiscard]] bool Build(ID3D11Device* device, const TileGrid& grid, std::uint32_t seed);
        void Rebuild(ID3D11Device* device, const TileGrid& grid, std::uint32_t seed);

        [[nodiscard]] bool IsValid() const noexcept { return shaderResourceView_ != nullptr; }
        [[nodiscard]] ID3D11ShaderResourceView* ShaderResourceView() const noexcept { return shaderResourceView_.Get(); }
        [[nodiscard]] ID3D11SamplerState* SamplerState() const noexcept { return samplerState_.Get(); }

        [[nodiscard]] int AtlasWidth() const noexcept { return atlasWidth_; }
        [[nodiscard]] int AtlasHeight() const noexcept { return atlasHeight_; }
        [[nodiscard]] int GridWidth() const noexcept { return gridWidth_; }
        [[nodiscard]] int GridHeight() const noexcept { return gridHeight_; }
        [[nodiscard]] int CellTexSize() const noexcept { return cellTexSize_; }

    private:
        void ReleaseGpuResources() noexcept;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView_;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState_;

        int atlasWidth_ = 0;
        int atlasHeight_ = 0;
        int gridWidth_ = 0;
        int gridHeight_ = 0;
        int cellTexSize_ = kProceduralTileTexSize;
    };
}
