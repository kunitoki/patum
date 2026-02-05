/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#pragma once

#include <concepts>
#include <type_traits>
#include <string_view>

#include "type_traits.h"

namespace ptm {

//=================================================================================================

template <class T>
concept StringLike = std::is_convertible_v<T, std::string_view>;

//=================================================================================================

template <class T>
concept boolean_testable = requires(T&& v)
    {
        { static_cast<bool>(v) } -> std::convertible_to<bool>;
    };

//=================================================================================================

template <class T>
concept dereferenceable = requires(T&& v)
    {
        { *v };
    };

//=================================================================================================

template <class T, class U>
concept dereferenceable_comparable_with = requires(T&& v)
    {
        { *v } -> std::equality_comparable_with<U>;
    };

//=================================================================================================

template <class U, class... Ts>
concept all_equality_comparable_with = requires(Ts... values)
    {
        { std::conditional_t<(std::equality_comparable_with<U, decltype(values)> && ...),
            std::true_type,
            std::false_type>{} } -> std::same_as<std::true_type>;
    };

} // namespace ptm
