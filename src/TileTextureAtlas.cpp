#include "TileTextureAtlas.h"

#include <cstring>
#include <vector>

namespace TinyCitySim
{
    bool TileTextureAtlas::Build(ID3D11Device* device, const TileGrid& grid, std::uint32_t seed)
    {
        if (device == nullptr)
        {
            return false;
        }

        ReleaseGpuResources();

        gridWidth_ = grid.Width();
        gridHeight_ = grid.Height();
        cellTexSize_ = kProceduralTileTexSize;
        atlasWidth_ = gridWidth_ * cellTexSize_;
        atlasHeight_ = gridHeight_ * cellTexSize_;

        std::vector<std::uint8_t> atlasPixels(static_cast<size_t>(atlasWidth_) * static_cast<size_t>(atlasHeight_) * 4u, 0u);
        std::vector<std::uint8_t> cellPixels(static_cast<size_t>(cellTexSize_) * static_cast<size_t>(cellTexSize_) * 4u);

        for (int row = 0; row < gridHeight_; ++row)
        {
            for (int col = 0; col < gridWidth_; ++col)
            {
                const GardenTile& tile = grid.At(col, row);
                ProceduralTileTexture::GenerateCell(
                    tile.type,
                    col,
                    row,
                    seed,
                    cellTexSize_,
                    cellPixels);

                for (int y = 0; y < cellTexSize_; ++y)
                {
                    const int atlasY = row * cellTexSize_ + y;
                    const size_t srcRow = static_cast<size_t>(y * cellTexSize_ * 4);
                    const size_t dstRow = static_cast<size_t>(atlasY * atlasWidth_ * 4 + col * cellTexSize_ * 4);
                    std::memcpy(
                        atlasPixels.data() + dstRow,
                        cellPixels.data() + srcRow,
                        static_cast<size_t>(cellTexSize_ * 4));
                }
            }
        }

        D3D11_TEXTURE2D_DESC textureDesc{};
        textureDesc.Width = static_cast<UINT>(atlasWidth_);
        textureDesc.Height = static_cast<UINT>(atlasHeight_);
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA initialData{};
        initialData.pSysMem = atlasPixels.data();
        initialData.SysMemPitch = static_cast<UINT>(atlasWidth_ * 4);

        const HRESULT textureHr = device->CreateTexture2D(&textureDesc, &initialData, &texture_);
        if (FAILED(textureHr))
        {
            ReleaseGpuResources();
            return false;
        }

        const HRESULT srvHr = device->CreateShaderResourceView(texture_.Get(), nullptr, &shaderResourceView_);
        if (FAILED(srvHr))
        {
            ReleaseGpuResources();
            return false;
        }

        D3D11_SAMPLER_DESC samplerDesc{};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        const HRESULT samplerHr = device->CreateSamplerState(&samplerDesc, &samplerState_);
        if (FAILED(samplerHr))
        {
            ReleaseGpuResources();
            return false;
        }

        return true;
    }

    void TileTextureAtlas::Rebuild(ID3D11Device* device, const TileGrid& grid, std::uint32_t seed)
    {
        (void)Build(device, grid, seed);
    }

    void TileTextureAtlas::ReleaseGpuResources() noexcept
    {
        samplerState_.Reset();
        shaderResourceView_.Reset();
        texture_.Reset();
        atlasWidth_ = 0;
        atlasHeight_ = 0;
        gridWidth_ = 0;
        gridHeight_ = 0;
    }
}
