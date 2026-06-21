#pragma once

#include "Platform.h"
#include <Windows.h>
#include <functional>

namespace TinyCitySim
{
    class Window
    {
    public:
        using ResizeCallback = std::function<void(int width, int height)>;
        using MouseMoveCallback = std::function<void(int x, int y)>;
        using MouseClickCallback = std::function<void(int x, int y)>;
        using CloseCallback = std::function<void()>;

        Window() = default;
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        [[nodiscard]] bool Create(
            HINSTANCE instance,
            int width,
            int height,
            const wchar_t* title);

        [[nodiscard]] HWND Handle() const noexcept { return hwnd_; }

        void SetResizeCallback(ResizeCallback callback);
        void SetMouseMoveCallback(MouseMoveCallback callback);
        void SetMouseClickCallback(MouseClickCallback callback);
        void SetCloseCallback(CloseCallback callback);

        [[nodiscard]] int ClientWidth() const noexcept { return clientWidth_; }
        [[nodiscard]] int ClientHeight() const noexcept { return clientHeight_; }

    private:
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

        void OnResize(int width, int height) noexcept;
        void OnMouseMove(int x, int y) noexcept;
        void OnMouseClick(int x, int y) noexcept;
        void OnClose() noexcept;

        HWND hwnd_ = nullptr;
        int clientWidth_ = 0;
        int clientHeight_ = 0;

        ResizeCallback resizeCallback_;
        MouseMoveCallback mouseMoveCallback_;
        MouseClickCallback mouseClickCallback_;
        CloseCallback closeCallback_;
    };
}
