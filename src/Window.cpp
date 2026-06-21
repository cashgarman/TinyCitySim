#include "Window.h"

#include <windowsx.h>

namespace TinyCitySim
{
    namespace
    {
        // Modern C++ (C++11): constexpr replaces #define for window class name.
        constexpr wchar_t kWindowClassName[] = L"TinyCitySimWindowClass";
    }

    Window::~Window()
    {
        if (hwnd_ != nullptr)
        {
            DestroyWindow(hwnd_);
            hwnd_ = nullptr;
        }
    }

    bool Window::Create(HINSTANCE instance, int width, int height, const wchar_t* title)
    {
        WNDCLASSEXW windowClass{};
        windowClass.cbSize = sizeof(WNDCLASSEXW);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = Window::WndProc;
        windowClass.hInstance = instance;
        windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        windowClass.lpszClassName = kWindowClassName;

        if (RegisterClassExW(&windowClass) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        {
            return false;
        }

        RECT windowRect{ 0, 0, width, height };
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

        const int windowWidth = windowRect.right - windowRect.left;
        const int windowHeight = windowRect.bottom - windowRect.top;

        hwnd_ = CreateWindowExW(
            0,
            kWindowClassName,
            title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            windowWidth,
            windowHeight,
            nullptr,
            nullptr,
            instance,
            this);

        if (hwnd_ == nullptr)
        {
            return false;
        }

        ShowWindow(hwnd_, SW_SHOW);
        UpdateWindow(hwnd_);

        RECT clientRect{};
        GetClientRect(hwnd_, &clientRect);
        clientWidth_ = clientRect.right - clientRect.left;
        clientHeight_ = clientRect.bottom - clientRect.top;

        return true;
    }

    void Window::SetResizeCallback(ResizeCallback callback)
    {
        resizeCallback_ = std::move(callback);
    }

    void Window::SetMouseMoveCallback(MouseMoveCallback callback)
    {
        mouseMoveCallback_ = std::move(callback);
    }

    void Window::SetMouseClickCallback(MouseClickCallback callback)
    {
        mouseClickCallback_ = std::move(callback);
    }

    void Window::SetCloseCallback(CloseCallback callback)
    {
        closeCallback_ = std::move(callback);
    }

    LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        Window* window = nullptr;

        if (message == WM_NCCREATE)
        {
            const auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
            window = static_cast<Window*>(createStruct->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        }
        else
        {
            window = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }

        switch (message)
        {
        case WM_SIZE:
        {
            if (window != nullptr)
            {
                const int width = LOWORD(lParam);
                const int height = HIWORD(lParam);
                window->OnResize(width, height);
            }
            return 0;
        }
        case WM_MOUSEMOVE:
        {
            if (window != nullptr)
            {
                const int x = GET_X_LPARAM(lParam);
                const int y = GET_Y_LPARAM(lParam);
                window->OnMouseMove(x, y);
            }
            return 0;
        }
        case WM_LBUTTONDOWN:
        {
            if (window != nullptr)
            {
                const int x = GET_X_LPARAM(lParam);
                const int y = GET_Y_LPARAM(lParam);
                window->OnMouseClick(x, y);
            }
            return 0;
        }
        case WM_DESTROY:
        {
            if (window != nullptr)
            {
                window->OnClose();
            }
            PostQuitMessage(0);
            return 0;
        }
        default:
            break;
        }

        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    void Window::OnResize(int width, int height) noexcept
    {
        clientWidth_ = width;
        clientHeight_ = height;

        if (resizeCallback_)
        {
            resizeCallback_(width, height);
        }
    }

    void Window::OnMouseMove(int x, int y) noexcept
    {
        if (mouseMoveCallback_)
        {
            mouseMoveCallback_(x, y);
        }
    }

    void Window::OnMouseClick(int x, int y) noexcept
    {
        if (mouseClickCallback_)
        {
            mouseClickCallback_(x, y);
        }
    }

    void Window::OnClose() noexcept
    {
        if (closeCallback_)
        {
            closeCallback_();
        }
    }
}
