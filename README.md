# TinyCitySim

A **DirectX 11 + Win32** tile-grid boilerplate for studying the kind of stack used to maintain long-lived Windows game clients (e.g. EverQuest's DX11 port). Hover a tile to highlight it; left-click to append its grid coordinates to an on-screen log.

## Prerequisites

- **Windows 10/11**
- **Visual Studio 2022** with:
  - Desktop development with C++
  - Windows 10/11 SDK

## Build

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
.\build\Debug\TinyCitySim.exe
```

Release build:

```powershell
cmake --build build --config Release
.\build\Release\TinyCitySim.exe
```

Shaders are copied to the output directory automatically by CMake (`shaders/` next to the `.exe`) for both Debug and Release builds.

In Cursor/VS Code, use **Run and Debug** with the **Debug TinyCitySim** launch configuration (F5) to build and run the Debug configuration.

## Controls

| Action | Result |
|---|---|
| **Mouse hover** | Highlights the tile under the cursor |
| **Left click** | Logs `Clicked tile (col, row)` in the overlay panel |
| **Resize window** | Grid re-centers; swap chain and projection update |

## Project structure

```
src/
  main.cpp           wWinMain entry, DPI awareness
  Application.*      Message pump + frame loop
  Window.*           Win32 window and input routing
  D3D11Context.*     D3D11 device, swap chain, D2D interop
  TileGrid.*         Grid layout and screen-to-tile hit testing
  TileRenderer.*     HLSL shader pipeline, colored quads
  InputHandler.*     Mouse → tile coordinate mapping
  LogPanel.*         DirectWrite/Direct2D click log overlay
  OrthoMath.h         Orthographic projection helpers (renamed to avoid shadowing <math.h>)
  ModernCppGuide.h   Study cheat sheet for modern C++ idioms used here
shaders/
  TileVertex.hlsl
  TilePixel.hlsl
```

## Modern C++ study notes

This project uses **C++20** with idioms commonly adopted in **new code** inside **legacy codebases**:

| Modern feature | Replaces (legacy style) |
|---|---|
| `ComPtr<T>` (WRL) | Raw COM pointers + manual `Release()` |
| `std::optional<TileCoord>` | Sentinel values like `(-1, -1)` |
| `std::format` | `swprintf` / string concatenation |
| `std::span<const Vertex>` | Raw pointer + vertex count |
| `constexpr` | `#define` constants |
| `= delete` copy | Accidental copies of device-owning objects |
| `[[nodiscard]]` | Ignored HRESULTs / error returns |

See [`src/ModernCppGuide.h`](src/ModernCppGuide.h) for the full list. Source files include inline `// Modern C++ (C++XX): ...` comments explaining each idiom and its legacy alternative.

**Windows gotcha:** Do not name a project header `Math.h` — on case-insensitive NTFS it shadows `<math.h>` and breaks STL compilation. This project uses `OrthoMath.h` instead.

## EverQuest interview talking points

1. **Why DirectX 11?** EQ migrated from DX9 to DX11 to stay compatible with modern GPU drivers while avoiding the larger rewrite cost of DX12/Vulkan for a mature client.
2. **Win32 message pump + render loop** — `PeekMessage` interleaved with `Present` is still how many legacy clients run; coroutines/async are newer and unevenly adopted.
3. **Screen-to-tile picking** — Same math as zone editors, debug overlays, and click-to-move tooling.
4. **Resize handling** — Swap-chain buffer recreation and D2D target rebinding are production concerns on varied monitors/DPI.
5. **Incremental modernization** — Write new modules in C++17/20 (`optional`, `ComPtr`, `format`); leave older modules on legacy patterns until refactored.
6. **Separation of concerns** — `TileGrid` (simulation/logic) vs `TileRenderer` (presentation) mirrors how large clients split gameplay from rendering.

## Font note

The log panel uses the system **Segoe UI** font via DirectWrite. No bundled font file is required on Windows 10/11.

## License

Boilerplate/study project — use freely for interview prep and learning.
