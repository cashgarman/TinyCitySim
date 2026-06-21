#pragma once

#include "Platform.h"

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <wrl/client.h>

namespace TinyCitySim
{
    class D3D11Context
    {
    public:
        D3D11Context() = default;
        ~D3D11Context() = default;

        D3D11Context(const D3D11Context&) = delete;
        D3D11Context& operator=(const D3D11Context&) = delete;

        [[nodiscard]] bool Initialize(HWND hwnd, int width, int height);
        [[nodiscard]] bool Resize(int width, int height);
        void BeginFrame(float clearR, float clearG, float clearB, float clearA);
        void EndFrame();

        [[nodiscard]] ID3D11Device* Device() const noexcept { return device_.Get(); }
        [[nodiscard]] ID3D11DeviceContext* DeviceContext() const noexcept { return context_.Get(); }
        [[nodiscard]] IDXGISwapChain* SwapChain() const noexcept { return swapChain_.Get(); }
        [[nodiscard]] ID2D1DeviceContext* D2DContext() const noexcept { return d2dContext_.Get(); }
        [[nodiscard]] IDWriteFactory* WriteFactory() const noexcept { return writeFactory_.Get(); }

        [[nodiscard]] int Width() const noexcept { return width_; }
        [[nodiscard]] int Height() const noexcept { return height_; }

    private:
        [[nodiscard]] bool CreateDeviceAndSwapChain(HWND hwnd, int width, int height);
        [[nodiscard]] bool CreateRenderTarget();
        [[nodiscard]] bool CreateD2DResources();
        void CleanupRenderTarget();

        // Modern C++ (RAII via WRL): ComPtr auto-Releases COM objects.
        // Legacy code used raw pointers with manual Release() and goto cleanup.
        Microsoft::WRL::ComPtr<ID3D11Device> device_;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
        Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;
        Microsoft::WRL::ComPtr<ID2D1Factory1> d2dFactory_;
        Microsoft::WRL::ComPtr<ID2D1Device> d2dDevice_;
        Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2dContext_;
        Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dTargetBitmap_;
        Microsoft::WRL::ComPtr<IDWriteFactory> writeFactory_;

        HWND hwnd_ = nullptr;
        int width_ = 0;
        int height_ = 0;
    };
}
