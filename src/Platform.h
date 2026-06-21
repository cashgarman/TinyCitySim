#pragma once

// Forced-include prelude: C runtime math must appear before any Windows/DirectX header.
#include <math.h>
#include <stdlib.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
