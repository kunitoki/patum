/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#pragma once

#include <utility>

namespace ptm {

//=================================================================================================

template <class... L>
struct overload_set : L...
{
    using L::operator()...;

    constexpr overload_set(L... lambda)
        : L(std::move(lambda))...
    {
    }
};

} // namespace ptm
