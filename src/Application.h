#pragma once

#include "Window.h"
#include "D3D11Context.h"
#include "TileGrid.h"
#include "TileRenderer.h"
#include "InputHandler.h"

#include <memory>
#include <optional>

namespace TinyCitySim
{
    class Application
    {
    public:
        Application() = default;
        ~Application() = default;

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        [[nodiscard]] bool Initialize(HINSTANCE instance);
        [[nodiscard]] int Run();

    private:
        void OnResize(int width, int height);
        void OnMouseMove(int x, int y) noexcept;
        void OnMouseClick(int x, int y);
        void OnClose() noexcept;
        void RenderFrame();

        // Modern C++ (C++14): std::make_unique for exception-safe ownership.
        std::unique_ptr<Window> window_;
        std::unique_ptr<D3D11Context> d3dContext_;
        std::unique_ptr<TileGrid> tileGrid_;
        std::unique_ptr<TileRenderer> tileRenderer_;
        std::unique_ptr<InputHandler> inputHandler_;

        bool running_ = true;
    };
}
