#pragma once

// Modern C++ study reference for TinyCitySim.
// Each idiom below is used in this project with inline comments explaining
// the legacy alternative common in long-lived game codebases (e.g. EverQuest).

/*
 * C++11  auto              - Type deduction; reduces verbose iterator/ComPtr types
 * C++11  nullptr           - Replaces NULL / 0 for pointer checks
 * C++11  constexpr         - Replaces #define for compile-time constants
 * C++11  enum class        - Strongly typed enums; no implicit int conversion
 * C++11  explicit          - Prevents unintended implicit conversions
 * C++11  override          - Documents virtual overrides; catches signature mistakes
 * C++11  = delete          - Makes types non-copyable/non-movable explicitly
 * C++11  = default         - Compiler-generated special members where intended
 * C++11  noexcept          - Documents functions that do not throw
 * C++11  [[nodiscard]]     - Warns when return values (especially HRESULTs) are ignored
 * C++11  range-for         - Replaces index loops over containers
 * C++11  std::optional     - Replaces sentinel values (-1) or bool + out-params
 * C++11  std::unique_ptr   - RAII for non-COM owned resources
 * C++14  std::make_unique  - Safer than raw new for unique_ptr construction
 * C++17  structured bindings - auto [a, b] = pair/struct; cleaner than .first/.second
 * C++17  std::wstring_view - Non-owning string reference; no allocation for read-only text
 * C++20  std::format       - Type-safe formatting; replaces swprintf / concatenation
 * C++20  designated init   - .Field = value for structs (D3D11 descs, viewports)
 * C++20  std::span         - Non-owning view over contiguous memory; replaces ptr+count
 * WRL    ComPtr<T>         - RAII for COM objects; replaces manual Release() calls
 *
 * Windows gotcha: never name project headers Math.h — it shadows <math.h> on case-insensitive
 * filesystems and breaks the entire STL (MSVC include path searches project dirs first).
 */

namespace ModernCppGuide
{
    inline constexpr int kReferenceVersion = 1;
}
