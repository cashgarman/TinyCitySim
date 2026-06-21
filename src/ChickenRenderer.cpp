#include "ChickenRenderer.h"

#include <d3dcompiler.h>
#include <fstream>
#include <vector>
#include <array>
#include <cstring>
#include <stdexcept>

namespace TinyCitySim
{
    namespace
    {
        inline constexpr float kChickenDrawSize = 24.0f;

        std::vector<std::byte> ReadBinaryFile(const wchar_t* path)
        {
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file)
            {
                throw std::runtime_error("Failed to open shader file");
            }

            const std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<std::byte> buffer(static_cast<size_t>(size));
            file.read(reinterpret_cast<char*>(buffer.data()), size);
            return buffer;
        }

        std::wstring GetExecutableDirectory()
        {
            std::wstring path(MAX_PATH, L'\0');
            const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
            path.resize(length);

            const size_t slash = path.find_last_of(L"\\/");
            if (slash != std::wstring::npos)
            {
                path.resize(slash + 1);
            }

            return path;
        }
    }

    bool ChickenRenderer::Initialize(ID3D11Device* device, int clientWidth, int clientHeight)
    {
        device_ = device;
        device_->GetImmediateContext(&context_);

        clientWidth_ = clientWidth;
        clientHeight_ = clientHeight;

        if (!LoadShaders(device))
        {
            return false;
        }

        if (!CreateConstantBuffers(device))
        {
            return false;
        }

        D3D11_BLEND_DESC blendDesc{};
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        const HRESULT blendHr = device_->CreateBlendState(&blendDesc, &alphaBlendState_);
        if (FAILED(blendHr))
        {
            return false;
        }

        D3D11_BUFFER_DESC dynamicBufferDesc{};
        dynamicBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * 6);
        dynamicBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        dynamicBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        dynamicBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        const HRESULT hr = device_->CreateBuffer(&dynamicBufferDesc, nullptr, &dynamicVertexBuffer_);
        return SUCCEEDED(hr);
    }

    bool ChickenRenderer::BuildAtlas(std::uint32_t seed)
    {
        atlasReady_ = atlas_.Build(device_.Get(), seed);
        return atlasReady_;
    }

    bool ChickenRenderer::LoadShaders(ID3D11Device* device)
    {
        const std::wstring shaderDirectory = GetExecutableDirectory() + L"shaders\\";
        const std::vector<std::byte> vertexSource = ReadBinaryFile((shaderDirectory + L"ChickenVertex.hlsl").c_str());
        const std::vector<std::byte> pixelSource = ReadBinaryFile((shaderDirectory + L"ChickenPixel.hlsl").c_str());

        Microsoft::WRL::ComPtr<ID3DBlob> vertexBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> pixelBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

        HRESULT hr = D3DCompile(
            vertexSource.data(),
            vertexSource.size(),
            "ChickenVertex.hlsl",
            nullptr,
            nullptr,
            "main",
            "vs_5_0",
            0,
            0,
            &vertexBlob,
            &errorBlob);

        if (FAILED(hr))
        {
            return false;
        }

        hr = D3DCompile(
            pixelSource.data(),
            pixelSource.size(),
            "ChickenPixel.hlsl",
            nullptr,
            nullptr,
            "main",
            "ps_5_0",
            0,
            0,
            &pixelBlob,
            &errorBlob);

        if (FAILED(hr))
        {
            return false;
        }

        hr = device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, &vertexShader_);
        if (FAILED(hr))
        {
            return false;
        }

        hr = device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, &pixelShader_);
        if (FAILED(hr))
        {
            return false;
        }

        const D3D11_INPUT_ELEMENT_DESC inputElements[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        hr = device->CreateInputLayout(
            inputElements,
            static_cast<UINT>(std::size(inputElements)),
            vertexBlob->GetBufferPointer(),
            vertexBlob->GetBufferSize(),
            &inputLayout_);

        return SUCCEEDED(hr);
    }

    bool ChickenRenderer::CreateConstantBuffers(ID3D11Device* device)
    {
        D3D11_BUFFER_DESC projectionBufferDesc{};
        projectionBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Math::Float4x4));
        projectionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        projectionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        projectionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT hr = device->CreateBuffer(&projectionBufferDesc, nullptr, &projectionConstantBuffer_);
        if (FAILED(hr))
        {
            return false;
        }

        D3D11_BUFFER_DESC spriteBufferDesc{};
        spriteBufferDesc.ByteWidth = static_cast<UINT>(sizeof(SpriteConstantBuffer));
        spriteBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        spriteBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        spriteBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        hr = device->CreateBuffer(&spriteBufferDesc, nullptr, &spriteConstantBuffer_);
        return SUCCEEDED(hr);
    }

    void ChickenRenderer::Resize(int clientWidth, int clientHeight)
    {
        clientWidth_ = clientWidth;
        clientHeight_ = clientHeight;
    }

    void ChickenRenderer::UpdateProjection(int clientWidth, int clientHeight)
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource{};
        if (FAILED(context_->Map(projectionConstantBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
        {
            return;
        }

        const Math::Float4x4 projection = Math::OrthographicProjection(
            static_cast<float>(clientWidth),
            static_cast<float>(clientHeight));

        std::memcpy(mappedResource.pData, &projection, sizeof(Math::Float4x4));
        context_->Unmap(projectionConstantBuffer_.Get(), 0);
    }

    void ChickenRenderer::UpdateSpriteConstants()
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource{};
        if (FAILED(context_->Map(spriteConstantBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
        {
            return;
        }

        SpriteConstantBuffer constants{};
        constants.frameCount = static_cast<float>(atlas_.FrameCount());
        constants.frameWidth = static_cast<float>(atlas_.FrameSize());
        constants.atlasWidth = static_cast<float>(kChickenAtlasWidth);

        std::memcpy(mappedResource.pData, &constants, sizeof(SpriteConstantBuffer));
        context_->Unmap(spriteConstantBuffer_.Get(), 0);
    }

    void ChickenRenderer::DrawSprite(float centerX, float centerY, float size, int frameIndex, bool faceLeft)
    {
        const float halfSize = size * 0.5f;
        const float x = centerX - halfSize;
        const float y = centerY - halfSize;
        const float frameU0 = static_cast<float>(frameIndex * kChickenFrameSize) / static_cast<float>(kChickenAtlasWidth);
        const float frameU1 = static_cast<float>((frameIndex + 1) * kChickenFrameSize) / static_cast<float>(kChickenAtlasWidth);
        const float flipX = faceLeft ? -1.0f : 1.0f;

        const std::array<Vertex, 6> vertices =
        {
            Vertex{ { x, y }, { frameU0, 0.0f }, flipX },
            Vertex{ { x + size, y }, { frameU1, 0.0f }, flipX },
            Vertex{ { x, y + size }, { frameU0, 1.0f }, flipX },
            Vertex{ { x + size, y }, { frameU1, 0.0f }, flipX },
            Vertex{ { x + size, y + size }, { frameU1, 1.0f }, flipX },
            Vertex{ { x, y + size }, { frameU0, 1.0f }, flipX },
        };

        D3D11_MAPPED_SUBRESOURCE mappedResource{};
        if (FAILED(context_->Map(dynamicVertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
        {
            return;
        }

        std::memcpy(mappedResource.pData, vertices.data(), sizeof(vertices));
        context_->Unmap(dynamicVertexBuffer_.Get(), 0);

        const UINT stride = static_cast<UINT>(sizeof(Vertex));
        const UINT offset = 0;
        context_->IASetVertexBuffers(0, 1, dynamicVertexBuffer_.GetAddressOf(), &stride, &offset);
        context_->IASetInputLayout(inputLayout_.Get());
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->VSSetShader(vertexShader_.Get(), nullptr, 0);
        context_->PSSetShader(pixelShader_.Get(), nullptr, 0);
        context_->VSSetConstantBuffers(0, 1, projectionConstantBuffer_.GetAddressOf());
        context_->PSSetConstantBuffers(0, 1, spriteConstantBuffer_.GetAddressOf());

        ID3D11ShaderResourceView* srv = atlas_.ShaderResourceView();
        context_->PSSetShaderResources(0, 1, &srv);

        ID3D11SamplerState* sampler = atlas_.SamplerState();
        context_->PSSetSamplers(0, 1, &sampler);

        const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        context_->OMSetBlendState(alphaBlendState_.Get(), blendFactor, 0xFFFFFFFFu);

        context_->Draw(static_cast<UINT>(vertices.size()), 0);
    }

    void ChickenRenderer::Draw(const ChickenSimulation& simulation, const TileGrid& grid, float elapsedTime)
    {
        (void)grid;
        (void)elapsedTime;

        if (!atlasReady_)
        {
            return;
        }

        UpdateProjection(clientWidth_, clientHeight_);
        UpdateSpriteConstants();

        for (const Chicken& chicken : simulation.Chickens())
        {
            const bool faceLeft = chicken.facing == ChickenFacing::Left;
            DrawSprite(chicken.posX, chicken.posY, kChickenDrawSize, chicken.animFrame, faceLeft);
        }

        context_->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFFu);
    }
}
