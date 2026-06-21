#include "Application.h"

#include "Platform.h"

#include <Windows.h>
#include <shellscalingapi.h>

#pragma comment(lib, "Shcore.lib")

// Modern C++ (Win32 entry): wWinMain is the Unicode Win32 entry point.
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE /*previousInstance*/, PWSTR /*commandLine*/, int /*showCommand*/)
{
    // Modern C++ (Win32): Per-monitor DPI awareness for crisp rendering on high-DPI displays.
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    TinyCitySim::Application application;

    if (!application.Initialize(instance))
    {
        MessageBoxW(
            nullptr,
            L"Failed to initialize TinyCitySim. Ensure shaders are copied next to the executable.",
            L"TinyCitySim",
            MB_OK | MB_ICONERROR);
        return -1;
    }

    return application.Run();
}
