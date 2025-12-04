/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#pragma once

#include <concepts>
#include <utility>

#include "predicate.h"

namespace ptm {

//=================================================================================================

template <class F>
struct wildcard : predicate<F>
{
    constexpr wildcard(F&& func)
        : predicate<F>(std::forward<F>(func))
    {
    }

    template <class T>
    friend constexpr bool operator==(const wildcard& lhs, const T& rhs) noexcept { return true; }

    template <class T>
        requires(not std::same_as<T, wildcard>)
    friend constexpr bool operator==(const T& lhs, const wildcard& rhs) noexcept { return true; }

    template <class T>
    friend constexpr bool operator!=(const wildcard& lhs, const T& rhs) noexcept { return false; }

    template <class T>
        requires(not std::same_as<T, wildcard>)
    friend constexpr bool operator!=(const T& lhs, const wildcard& rhs) noexcept { return true; }

    template <class T>
    friend constexpr bool operator<(const wildcard& lhs, const T& rhs) noexcept { return true; }

    template <class T>
        requires(not std::same_as<T, wildcard>)
    friend constexpr bool operator<(const T& lhs, const wildcard& rhs) noexcept { return true; }

    template <class T>
    friend constexpr bool operator<=(const wildcard& lhs, const T& rhs) noexcept { return true; }

    template <class T>
        requires(not std::same_as<T, wildcard>)
    friend constexpr bool operator<=(const T& lhs, const wildcard& rhs) noexcept { return true; }

    template <class T>
    friend constexpr bool operator>(const wildcard& lhs, const T& rhs) noexcept { return true; }

    template <class T>
        requires(not std::same_as<T, wildcard>)
    friend constexpr bool operator>(const T& lhs, const wildcard& rhs) noexcept { return true; }

    template <class T>
    friend constexpr bool operator>=(const wildcard& lhs, const T& rhs) noexcept { return true; }

    template <class T>
        requires(not std::same_as<T, wildcard>)
    friend constexpr bool operator>=(const T& lhs, const wildcard& rhs) noexcept { return true; }
};

//=================================================================================================

inline static constexpr auto _ = wildcard([]<class U>([[maybe_unused]] const U& value_to_test)
{
    return true;
});

} // namespace ptm
