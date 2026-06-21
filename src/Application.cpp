#include "Application.h"

#include "GardenGenerator.h"
#include "LogPanel.h"

namespace TinyCitySim
{
    namespace
    {
        inline constexpr int kInitialWindowWidth = 960;
        inline constexpr int kInitialWindowHeight = 720;
        inline constexpr wchar_t kWindowTitle[] = L"TinyCitySim - DX11 Tile Grid";
    }

    bool Application::Initialize(HINSTANCE instance)
    {
        // Modern C++ (C++14): std::make_unique for exception-safe heap ownership.
        window_ = std::make_unique<Window>();
        d3dContext_ = std::make_unique<D3D11Context>();
        tileGrid_ = std::make_unique<TileGrid>(kDefaultGridWidth, kDefaultGridHeight, kDefaultTileSize);

        // Generation runs at init (data layer), not in the renderer — TileGrid owns tile content.
        GardenGenerator generator{ 42u };
        generator.Generate(*tileGrid_);

        tileRenderer_ = std::make_unique<TileRenderer>();
        inputHandler_ = std::make_unique<InputHandler>(*tileGrid_);

        if (!window_->Create(instance, kInitialWindowWidth, kInitialWindowHeight, kWindowTitle))
        {
            return false;
        }

        if (!d3dContext_->Initialize(window_->Handle(), window_->ClientWidth(), window_->ClientHeight()))
        {
            return false;
        }

        if (!tileRenderer_->Initialize(d3dContext_->Device(), window_->ClientWidth(), window_->ClientHeight()))
        {
            return false;
        }

        LogPanel::Instance().Initialize(d3dContext_->D2DContext(), d3dContext_->WriteFactory());

        if (!tileRenderer_->BuildAtlas(*tileGrid_, 42u))
        {
            // Failure message logged by TileRenderer.
        }

        tileGrid_->SetClientSize(window_->ClientWidth(), window_->ClientHeight());

        window_->SetResizeCallback([this](int width, int height)
        {
            OnResize(width, height);
        });

        window_->SetMouseMoveCallback([this](int x, int y)
        {
            OnMouseMove(x, y);
        });

        window_->SetMouseClickCallback([this](int x, int y)
        {
            OnMouseClick(x, y);
        });

        window_->SetCloseCallback([this]()
        {
            OnClose();
        });

        return true;
    }

    int Application::Run()
    {
        MSG message{};

        while (running_)
        {
            while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
            {
                if (message.message == WM_QUIT)
                {
                    running_ = false;
                    break;
                }

                TranslateMessage(&message);
                DispatchMessageW(&message);
            }

            if (running_)
            {
                RenderFrame();
            }
        }

        return static_cast<int>(message.wParam);
    }

    void Application::OnResize(int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            return;
        }

        if (!d3dContext_->Resize(width, height))
        {
            return;
        }
        tileRenderer_->Resize(width, height);
        tileGrid_->SetClientSize(width, height);
    }

    void Application::OnMouseMove(int x, int y) noexcept
    {
        inputHandler_->OnMouseMove(x, y);
    }

    void Application::OnMouseClick(int x, int y)
    {
        inputHandler_->OnMouseClick(x, y);
    }

    void Application::OnClose() noexcept
    {
        running_ = false;
    }

    void Application::RenderFrame()
    {
        d3dContext_->BeginFrame(0.05f, 0.07f, 0.12f, 1.0f);
        tileRenderer_->Draw(*tileGrid_, inputHandler_->HoveredTile());
        LogPanel::Instance().Draw();
        d3dContext_->EndFrame();
    }
}
