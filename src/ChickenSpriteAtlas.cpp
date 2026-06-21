#include "ChickenSpriteAtlas.h"

#include <vector>

namespace TinyCitySim
{
    bool ChickenSpriteAtlas::Build(ID3D11Device* device, std::uint32_t seed)
    {
        if (device == nullptr)
        {
            return false;
        }

        ReleaseGpuResources();

        std::vector<std::uint8_t> pixels(static_cast<size_t>(kChickenAtlasWidth * kChickenFrameSize * 4), 0u);
        ProceduralChickenSprite::GenerateAtlas(seed, pixels);

        D3D11_TEXTURE2D_DESC textureDesc{};
        textureDesc.Width = static_cast<UINT>(kChickenAtlasWidth);
        textureDesc.Height = static_cast<UINT>(kChickenFrameSize);
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA initialData{};
        initialData.pSysMem = pixels.data();
        initialData.SysMemPitch = static_cast<UINT>(kChickenAtlasWidth * 4);

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
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
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

    void ChickenSpriteAtlas::ReleaseGpuResources() noexcept
    {
        samplerState_.Reset();
        shaderResourceView_.Reset();
        texture_.Reset();
    }
}
