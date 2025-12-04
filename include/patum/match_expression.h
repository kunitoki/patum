/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#pragma once

#include <string_view>

#include "concepts.h"

namespace ptm {

//=================================================================================================

template <class E>
struct match_expression
{
    constexpr explicit match_expression(const E& expression)
        : expression_(expression)
    {
    }

    template <class T> requires StringLike<T>
    constexpr explicit match_expression(const T& expression)
        : expression_(std::string_view(expression))
    {
    }

    constexpr decltype(auto) get() const
    {
        return expression_;
    }

private:
    std::conditional_t<
        StringLike<E>,
        std::string_view,
        const E&> expression_;
};

} // namespace ptm
