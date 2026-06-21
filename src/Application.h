#pragma once

#include "Window.h"
#include "D3D11Context.h"
#include "TileGrid.h"
#include "TileRenderer.h"
#include "InputHandler.h"
#include "GrassSimulation.h"
#include "Chicken.h"
#include "ChickenRenderer.h"

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
        void UpdateSimulation(float dt);
        void RenderFrame();

        std::unique_ptr<Window> window_;
        std::unique_ptr<D3D11Context> d3dContext_;
        std::unique_ptr<TileGrid> tileGrid_;
        std::unique_ptr<TileRenderer> tileRenderer_;
        std::unique_ptr<GrassSimulation> grassSimulation_;
        std::unique_ptr<ChickenSimulation> chickenSimulation_;
        std::unique_ptr<ChickenRenderer> chickenRenderer_;
        std::unique_ptr<InputHandler> inputHandler_;

        bool running_ = true;
        float elapsedTime_ = 0.0f;
        long long lastFrameTicks_ = 0;
        long long perfFrequency_ = 0;
    };
}
