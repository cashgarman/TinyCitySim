#include "D3D11Context.h"

#include <stdexcept>
#include <string>

namespace TinyCitySim
{
    namespace
    {
        void ThrowIfFailed(HRESULT hr, const char* message)
        {
            if (FAILED(hr))
            {
                throw std::runtime_error(message);
            }
        }
    }

    bool D3D11Context::Initialize(HWND hwnd, int width, int height)
    {
        hwnd_ = hwnd;
        width_ = width;
        height_ = height;

        try
        {
            if (!CreateDeviceAndSwapChain(hwnd, width, height))
            {
                return false;
            }

            if (!CreateRenderTarget())
            {
                return false;
            }

            if (!CreateD2DResources())
            {
                return false;
            }

            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool D3D11Context::CreateDeviceAndSwapChain(HWND hwnd, int width, int height)
    {
        UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
        };

        D3D_FEATURE_LEVEL createdFeatureLevel = D3D_FEATURE_LEVEL_11_0;

        Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
        ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)), "CreateDXGIFactory1 failed");

        DXGI_SWAP_CHAIN_DESC swapChainDesc{};
        swapChainDesc.BufferCount = 2;
        swapChainDesc.BufferDesc.Width = static_cast<UINT>(width);
        swapChainDesc.BufferDesc.Height = static_cast<UINT>(height);
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = hwnd;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            featureLevels,
            static_cast<UINT>(std::size(featureLevels)),
            D3D11_SDK_VERSION,
            &swapChainDesc,
            &swapChain,
            &device_,
            &createdFeatureLevel,
            &context_);

        if (FAILED(hr))
        {
            return false;
        }

        swapChain_ = swapChain;
        return true;
    }

    bool D3D11Context::CreateRenderTarget()
    {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
        ThrowIfFailed(swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer)), "GetBuffer failed");

        ThrowIfFailed(
            device_->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView_),
            "CreateRenderTargetView failed");

        // Modern C++ (C++20): designated initializers for D3D viewport setup.
        D3D11_VIEWPORT viewport
        {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<float>(width_),
            .Height = static_cast<float>(height_),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,
        };

        context_->RSSetViewports(1, &viewport);
        return true;
    }

    bool D3D11Context::CreateD2DResources()
    {
        ThrowIfFailed(
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&d2dFactory_)),
            "D2D1CreateFactory failed");

        ThrowIfFailed(
            DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(writeFactory_.GetAddressOf())),
            "DWriteCreateFactory failed");

        Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
        ThrowIfFailed(device_.As(&dxgiDevice), "QueryInterface IDXGIDevice failed");

        ThrowIfFailed(
            d2dFactory_->CreateDevice(dxgiDevice.Get(), &d2dDevice_),
            "CreateDevice failed");

        ThrowIfFailed(
            d2dDevice_->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dContext_),
            "CreateDeviceContext failed");

        Microsoft::WRL::ComPtr<IDXGISurface> dxgiSurface;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
        ThrowIfFailed(swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer)), "GetBuffer failed");
        ThrowIfFailed(backBuffer.As(&dxgiSurface), "QueryInterface IDXGISurface failed");

        D2D1_BITMAP_PROPERTIES1 bitmapProperties
        {
            .pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            .dpiX = 96.0f,
            .dpiY = 96.0f,
            .bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            .colorContext = nullptr,
        };

        ThrowIfFailed(
            d2dContext_->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), &bitmapProperties, &d2dTargetBitmap_),
            "CreateBitmapFromDxgiSurface failed");

        d2dContext_->SetTarget(d2dTargetBitmap_.Get());
        return true;
    }

    void D3D11Context::CleanupRenderTarget()
    {
        if (d2dContext_)
        {
            d2dContext_->SetTarget(nullptr);
        }

        d2dTargetBitmap_.Reset();
        renderTargetView_.Reset();
    }

    bool D3D11Context::Resize(int width, int height)
    {
        if (width_ == width && height_ == height)
        {
            return true;
        }

        if (width <= 0 || height <= 0)
        {
            return false;
        }

        try
        {
            width_ = width;
            height_ = height;

            CleanupRenderTarget();

            ThrowIfFailed(
                swapChain_->ResizeBuffers(0, static_cast<UINT>(width), static_cast<UINT>(height), DXGI_FORMAT_UNKNOWN, 0),
                "ResizeBuffers failed");

            if (!CreateRenderTarget())
            {
                return false;
            }

            Microsoft::WRL::ComPtr<IDXGISurface> dxgiSurface;
            Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
            ThrowIfFailed(swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer)), "GetBuffer failed");
            ThrowIfFailed(backBuffer.As(&dxgiSurface), "QueryInterface IDXGISurface failed");

            D2D1_BITMAP_PROPERTIES1 bitmapProperties
            {
                .pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                .dpiX = 96.0f,
                .dpiY = 96.0f,
                .bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                .colorContext = nullptr,
            };

            ThrowIfFailed(
                d2dContext_->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), &bitmapProperties, &d2dTargetBitmap_),
                "CreateBitmapFromDxgiSurface failed");

            d2dContext_->SetTarget(d2dTargetBitmap_.Get());
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    void D3D11Context::BeginFrame(float clearR, float clearG, float clearB, float clearA)
    {
        const float clearColor[4] = { clearR, clearG, clearB, clearA };
        context_->ClearRenderTargetView(renderTargetView_.Get(), clearColor);
        context_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), nullptr);

        if (d2dContext_)
        {
            d2dContext_->BeginDraw();
        }
    }

    void D3D11Context::EndFrame()
    {
        if (d2dContext_)
        {
            d2dContext_->EndDraw();
        }

        swapChain_->Present(1, 0);
    }
}
