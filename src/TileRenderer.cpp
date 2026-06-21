#include "TileRenderer.h"

#include "GardenTile.h"
#include "LogPanel.h"

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
        inline constexpr Math::Float4 kTileHoverColor{ 0.35f, 0.55f, 0.35f, 1.0f };
        inline constexpr Math::Float4 kTileBorderColor{ 0.85f, 0.95f, 0.55f, 1.0f };
        inline constexpr float kBorderWidth = 2.0f;

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

    bool TileRenderer::Initialize(ID3D11Device* device, int clientWidth, int clientHeight)
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

        D3D11_BUFFER_DESC dynamicBufferDesc{};
        dynamicBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * 6);
        dynamicBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        dynamicBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        dynamicBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        const HRESULT hr = device_->CreateBuffer(&dynamicBufferDesc, nullptr, &dynamicVertexBuffer_);
        return SUCCEEDED(hr);
    }

    bool TileRenderer::BuildAtlas(const TileGrid& grid, std::uint32_t seed)
    {
        atlasReady_ = atlas_.Build(device_.Get(), grid, seed);
        if (!atlasReady_)
        {
            LogPanel::Instance().Log(L"Failed to build tile texture atlas; using solid colors.");
        }

        return atlasReady_;
    }

    bool TileRenderer::LoadShaders(ID3D11Device* device)
    {
        const std::wstring shaderDirectory = GetExecutableDirectory() + L"shaders\\";
        const std::vector<std::byte> vertexSource = ReadBinaryFile((shaderDirectory + L"TileVertex.hlsl").c_str());
        const std::vector<std::byte> pixelSource = ReadBinaryFile((shaderDirectory + L"TilePixel.hlsl").c_str());

        Microsoft::WRL::ComPtr<ID3DBlob> vertexBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> pixelBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

        HRESULT hr = D3DCompile(
            vertexSource.data(),
            vertexSource.size(),
            "TileVertex.hlsl",
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
            "TilePixel.hlsl",
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
            { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        hr = device->CreateInputLayout(
            inputElements,
            static_cast<UINT>(std::size(inputElements)),
            vertexBlob->GetBufferPointer(),
            vertexBlob->GetBufferSize(),
            &inputLayout_);

        return SUCCEEDED(hr);
    }

    bool TileRenderer::CreateConstantBuffers(ID3D11Device* device)
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

        D3D11_BUFFER_DESC atlasBufferDesc{};
        atlasBufferDesc.ByteWidth = static_cast<UINT>(sizeof(AtlasConstantBuffer));
        atlasBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        atlasBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        atlasBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        hr = device->CreateBuffer(&atlasBufferDesc, nullptr, &atlasConstantBuffer_);
        return SUCCEEDED(hr);
    }

    void TileRenderer::Resize(int clientWidth, int clientHeight)
    {
        clientWidth_ = clientWidth;
        clientHeight_ = clientHeight;
        UpdateProjection(clientWidth, clientHeight);
    }

    void TileRenderer::UpdateProjection(int clientWidth, int clientHeight)
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

    void TileRenderer::UpdateAtlasConstants(bool useSolidColor)
    {
        if (!atlasReady_)
        {
            return;
        }

        D3D11_MAPPED_SUBRESOURCE mappedResource{};
        if (FAILED(context_->Map(atlasConstantBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
        {
            return;
        }

        AtlasConstantBuffer atlasConstants{};
        atlasConstants.atlasGridSize = {
            static_cast<float>(atlas_.GridWidth()),
            static_cast<float>(atlas_.GridHeight()),
        };
        atlasConstants.atlasInvGridSize = {
            1.0f / static_cast<float>(atlas_.GridWidth()),
            1.0f / static_cast<float>(atlas_.GridHeight()),
        };
        atlasConstants.useSolidColor = useSolidColor ? 1.0f : 0.0f;
        atlasConstants.cellTexSize = static_cast<float>(atlas_.CellTexSize());

        std::memcpy(mappedResource.pData, &atlasConstants, sizeof(AtlasConstantBuffer));
        context_->Unmap(atlasConstantBuffer_.Get(), 0);
    }

    void TileRenderer::DrawQuad(
        float x,
        float y,
        float width,
        float height,
        const Math::Float2& tileCoord,
        const Math::Float4& color,
        bool useSolidColor)
    {
        const std::array<Vertex, 6> vertices =
        {
            Vertex{ { x, y }, { 0.0f, 0.0f }, tileCoord, color },
            Vertex{ { x + width, y }, { 1.0f, 0.0f }, tileCoord, color },
            Vertex{ { x, y + height }, { 0.0f, 1.0f }, tileCoord, color },
            Vertex{ { x + width, y }, { 1.0f, 0.0f }, tileCoord, color },
            Vertex{ { x + width, y + height }, { 1.0f, 1.0f }, tileCoord, color },
            Vertex{ { x, y + height }, { 0.0f, 1.0f }, tileCoord, color },
        };

        UpdateAtlasConstants(useSolidColor || !atlasReady_);

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
        context_->PSSetConstantBuffers(1, 1, atlasConstantBuffer_.GetAddressOf());

        if (atlasReady_ && !useSolidColor)
        {
            ID3D11ShaderResourceView* srv = atlas_.ShaderResourceView();
            context_->PSSetShaderResources(0, 1, &srv);

            ID3D11SamplerState* sampler = atlas_.SamplerState();
            context_->PSSetSamplers(0, 1, &sampler);
        }

        context_->Draw(static_cast<UINT>(vertices.size()), 0);
    }

    void TileRenderer::DrawTileFill(const TileGrid& grid, int col, int row, const Math::Float4& color, bool useSolidColor)
    {
        const float x = static_cast<float>(grid.OriginX() + col * grid.TileSize());
        const float y = static_cast<float>(grid.OriginY() + row * grid.TileSize());
        const float size = static_cast<float>(grid.TileSize());
        const Math::Float2 tileCoord = {
            static_cast<float>(col),
            static_cast<float>(row),
        };

        DrawQuad(x, y, size, size, tileCoord, color, useSolidColor);
    }

    void TileRenderer::DrawTileBorder(const TileGrid& grid, int col, int row, float borderWidth, const Math::Float4& color)
    {
        const float x = static_cast<float>(grid.OriginX() + col * grid.TileSize());
        const float y = static_cast<float>(grid.OriginY() + row * grid.TileSize());
        const float size = static_cast<float>(grid.TileSize());
        const Math::Float2 tileCoord = {
            static_cast<float>(col),
            static_cast<float>(row),
        };

        DrawQuad(x, y, size, borderWidth, tileCoord, color, true);
        DrawQuad(x, y + size - borderWidth, size, borderWidth, tileCoord, color, true);
        DrawQuad(x, y, borderWidth, size, tileCoord, color, true);
        DrawQuad(x + size - borderWidth, y, borderWidth, size, tileCoord, color, true);
    }

    void TileRenderer::Draw(const TileGrid& grid, const std::optional<TileCoord>& hoveredTile)
    {
        UpdateProjection(clientWidth_, clientHeight_);

        for (int row = 0; row < grid.Height(); ++row)
        {
            for (int col = 0; col < grid.Width(); ++col)
            {
                const GardenTile& tile = grid.At(col, row);
                const Math::Float4 fallbackColor = ColorFor(tile.type);
                const bool useSolidColor = !atlasReady_;
                DrawTileFill(grid, col, row, fallbackColor, useSolidColor);
            }
        }

        if (hoveredTile.has_value())
        {
            const auto [col, row] = *hoveredTile;
            DrawTileFill(grid, col, row, kTileHoverColor, true);
            DrawTileBorder(grid, col, row, kBorderWidth, kTileBorderColor);
        }
    }
}
