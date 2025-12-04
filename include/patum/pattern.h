/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#pragma once

#include <tuple>
#include <utility>

namespace ptm {

//=================================================================================================

template <class... Args>
struct match_pattern
{
    constexpr explicit match_pattern(Args&&... args)
        : args_(std::forward_as_tuple(args...))
    {
    }

    template <class T>
    constexpr matcher<T, Args...> operator=(T&& result) const &&
    {
        return { std::forward<T>(result), std::move(args_) };
    }

private:
    [[no_unique_address]] std::tuple<Args...> args_;
};

template <class... Args>
[[nodiscard]] constexpr match_pattern<Args...> pattern(Args&&... args)
{
    return match_pattern<Args...>{ std::forward<Args>(args)... };
}

} // namespace ptm
