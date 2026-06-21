#pragma once

#include "ProceduralChickenSprite.h"

#include "Platform.h"

#include <d3d11.h>
#include <wrl/client.h>

namespace TinyCitySim
{
    class ChickenSpriteAtlas
    {
    public:
        [[nodiscard]] bool Build(ID3D11Device* device, std::uint32_t seed);

        [[nodiscard]] bool IsValid() const noexcept { return shaderResourceView_ != nullptr; }
        [[nodiscard]] ID3D11ShaderResourceView* ShaderResourceView() const noexcept { return shaderResourceView_.Get(); }
        [[nodiscard]] ID3D11SamplerState* SamplerState() const noexcept { return samplerState_.Get(); }
        [[nodiscard]] int FrameCount() const noexcept { return kChickenFrameCount; }
        [[nodiscard]] int FrameSize() const noexcept { return kChickenFrameSize; }

    private:
        void ReleaseGpuResources() noexcept;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView_;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState_;
    };
}
