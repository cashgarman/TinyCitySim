#pragma once

#include "Platform.h"

#include <Windows.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <deque>
#include <string>
#include <string_view>
#include <wrl/client.h>

namespace TinyCitySim
{
    class LogPanel
    {
    public:
        // Modern C++ (Meyers singleton): simple global log for demo; production code would inject dependencies.
        static LogPanel& Instance();

        void Initialize(ID2D1DeviceContext* d2dContext, IDWriteFactory* writeFactory);
        void AddEntry(int col, int row, std::wstring_view tileName);
        void Log(std::wstring_view message);
        void Draw();

        LogPanel(const LogPanel&) = delete;
        LogPanel& operator=(const LogPanel&) = delete;

    private:
        LogPanel() = default;

        static constexpr int kMaxLines = 12;

        Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2dContext_;
        Microsoft::WRL::ComPtr<IDWriteFactory> writeFactory_;
        Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat_;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> backgroundBrush_;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> textBrush_;

        std::deque<std::wstring> logLines_;
        bool resourcesCreated_ = false;
    };
}
