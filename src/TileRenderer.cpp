#include "TileRenderer.h"

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
        // Modern C++ (C++11): constexpr color constants replace #define macros.
        inline constexpr Math::Float4 kTileBaseColor{ 0.18f, 0.28f, 0.18f, 1.0f };
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

        if (!CreateConstantBuffer(device))
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
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        hr = device->CreateInputLayout(
            inputElements,
            static_cast<UINT>(std::size(inputElements)),
            vertexBlob->GetBufferPointer(),
            vertexBlob->GetBufferSize(),
            &inputLayout_);

        return SUCCEEDED(hr);
    }

    bool TileRenderer::CreateConstantBuffer(ID3D11Device* device)
    {
        D3D11_BUFFER_DESC constantBufferDesc{};
        constantBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Math::Float4x4));
        constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        const HRESULT hr = device->CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer_);
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
        if (FAILED(context_->Map(constantBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
        {
            return;
        }

        const Math::Float4x4 projection = Math::OrthographicProjection(
            static_cast<float>(clientWidth),
            static_cast<float>(clientHeight));

        std::memcpy(mappedResource.pData, &projection, sizeof(Math::Float4x4));
        context_->Unmap(constantBuffer_.Get(), 0);
    }

    void TileRenderer::DrawQuad(float x, float y, float width, float height, const Math::Float4& color)
    {
        // Modern C++ (C++20): std::span wraps contiguous vertex data instead of raw pointer + count.
        const std::array<Vertex, 6> vertices =
        {
            Vertex{ { x, y }, color },
            Vertex{ { x + width, y }, color },
            Vertex{ { x, y + height }, color },
            Vertex{ { x + width, y }, color },
            Vertex{ { x + width, y + height }, color },
            Vertex{ { x, y + height }, color },
        };

        const std::span<const Vertex> vertexSpan = vertices;

        D3D11_MAPPED_SUBRESOURCE mappedResource{};
        if (FAILED(context_->Map(dynamicVertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
        {
            return;
        }

        std::memcpy(mappedResource.pData, vertexSpan.data(), vertexSpan.size_bytes());
        context_->Unmap(dynamicVertexBuffer_.Get(), 0);

        const UINT stride = static_cast<UINT>(sizeof(Vertex));
        const UINT offset = 0;
        context_->IASetVertexBuffers(0, 1, dynamicVertexBuffer_.GetAddressOf(), &stride, &offset);
        context_->IASetInputLayout(inputLayout_.Get());
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->VSSetShader(vertexShader_.Get(), nullptr, 0);
        context_->PSSetShader(pixelShader_.Get(), nullptr, 0);
        context_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
        context_->Draw(static_cast<UINT>(vertexSpan.size()), 0);
    }

    void TileRenderer::DrawTileFill(const TileGrid& grid, int col, int row, const Math::Float4& color)
    {
        const float x = static_cast<float>(grid.OriginX() + col * grid.TileSize());
        const float y = static_cast<float>(grid.OriginY() + row * grid.TileSize());
        const float size = static_cast<float>(grid.TileSize());
        DrawQuad(x, y, size, size, color);
    }

    void TileRenderer::DrawTileBorder(const TileGrid& grid, int col, int row, float borderWidth, const Math::Float4& color)
    {
        const float x = static_cast<float>(grid.OriginX() + col * grid.TileSize());
        const float y = static_cast<float>(grid.OriginY() + row * grid.TileSize());
        const float size = static_cast<float>(grid.TileSize());

        DrawQuad(x, y, size, borderWidth, color);
        DrawQuad(x, y + size - borderWidth, size, borderWidth, color);
        DrawQuad(x, y, borderWidth, size, color);
        DrawQuad(x + size - borderWidth, y, borderWidth, size, color);
    }

    void TileRenderer::Draw(const TileGrid& grid, const std::optional<TileCoord>& hoveredTile)
    {
        UpdateProjection(clientWidth_, clientHeight_);

        for (int row = 0; row < grid.Height(); ++row)
        {
            for (int col = 0; col < grid.Width(); ++col)
            {
                DrawTileFill(grid, col, row, kTileBaseColor);
            }
        }

        if (hoveredTile.has_value())
        {
            // Modern C++ (C++17): structured bindings after optional check.
            const auto [col, row] = *hoveredTile;
            DrawTileFill(grid, col, row, kTileHoverColor);
            DrawTileBorder(grid, col, row, kBorderWidth, kTileBorderColor);
        }
    }
}
