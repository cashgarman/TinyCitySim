#include "LogPanel.h"

#include <format>
#include <string_view>

namespace TinyCitySim
{
    LogPanel& LogPanel::Instance()
    {
        static LogPanel instance;
        return instance;
    }

    void LogPanel::Initialize(ID2D1DeviceContext* d2dContext, IDWriteFactory* writeFactory)
    {
        d2dContext_ = d2dContext;
        writeFactory_ = writeFactory;

        if (resourcesCreated_)
        {
            return;
        }

        writeFactory_->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            16.0f,
            L"en-us",
            &textFormat_);

        textFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        textFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

        d2dContext_->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.65f), &backgroundBrush_);
        d2dContext_->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &textBrush_);

        logLines_.push_back(L"Hover a tile to highlight it.");
        logLines_.push_back(L"Left-click a tile to log coordinates and type.");

        resourcesCreated_ = true;
    }

    void LogPanel::AddEntry(int col, int row, std::wstring_view tileName)
    {
        // Modern C++ (C++20): std::format replaces swprintf / manual wstring concatenation.
        logLines_.push_back(std::format(L"Clicked ({}, {}) — {}", col, row, tileName));

        while (static_cast<int>(logLines_.size()) > kMaxLines)
        {
            logLines_.pop_front();
        }
    }

    void LogPanel::Log(std::wstring_view message)
    {
        logLines_.push_back(std::wstring(message));

        while (static_cast<int>(logLines_.size()) > kMaxLines)
        {
            logLines_.pop_front();
        }
    }

    void LogPanel::Draw()
    {
        if (!d2dContext_ || !textFormat_ || !backgroundBrush_ || !textBrush_)
        {
            return;
        }

        const D2D1_RECT_F backgroundRect{ 12.0f, 12.0f, 360.0f, 240.0f };
        d2dContext_->FillRectangle(backgroundRect, backgroundBrush_.Get());

        float y = 20.0f;
        const float lineHeight = 18.0f;

        // Modern C++ (C++11): range-based for replaces index loop over deque.
        for (const auto& line : logLines_)
        {
            // Modern C++ (C++17): wstring_view avoids copying when passing to DrawText.
            const std::wstring_view lineView = line;
            const D2D1_RECT_F textRect{ 20.0f, y, 350.0f, y + lineHeight };
            d2dContext_->DrawText(
                lineView.data(),
                static_cast<UINT32>(lineView.size()),
                textFormat_.Get(),
                textRect,
                textBrush_.Get());
            y += lineHeight;
        }
    }
}
