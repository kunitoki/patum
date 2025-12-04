/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#pragma once

#include <cassert>

namespace ptm {

//=================================================================================================

constexpr void expect(bool condition)
{
    if (not condition)
    {
        assert(condition);
    }
}

} // namespace ptm
