/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#pragma once

//=================================================================================================

#if __has_include(<re2/re2.h>)
#include <re2/re2.h>
#define PATUM_HAS_FEATURE_RE2 1
#else
#define PATUM_HAS_FEATURE_RE2 0
#endif
