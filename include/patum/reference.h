/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#pragma once

#include "type_traits.h"

namespace ptm {

template <class T>
    requires(not dereferenceable<T>)
constexpr decltype(auto) dereference(T&& value) noexcept
{
    return std::forward<T>(value);
}

template <class T>
    requires dereferenceable<T>
constexpr decltype(auto) dereference(T& value) noexcept
{
    return *value;
}

template <class T>
    requires dereferenceable<T>
constexpr decltype(auto) dereference(T&& value) noexcept
{
    return *std::move(value);
}

template <class T>
    requires dereferenceable<T>
constexpr decltype(auto) dereference(const T& value) noexcept
{
    return *value;
}

template <class T>
    requires dereferenceable<T>
constexpr decltype(auto) dereference(const T&& value) noexcept
{
    return *std::move(value);
}

} // namespace ptm
