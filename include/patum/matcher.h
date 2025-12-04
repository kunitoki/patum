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
#include <tuple>
#include <utility>

#include "tuple.h"

namespace ptm {

//=================================================================================================

template <class T, class... Args>
struct matcher
{
    inline static constexpr std::size_t capture_count = sizeof...(Args);

    constexpr matcher(T&& result, std::tuple<Args...> args)
        : result_(std::move(result))
        , args_(std::move(args))
    {
    }

    template <class... U>
    constexpr bool check(const U&... values_to_test) const
    {
        return tuple_unpacker<sizeof...(U)>::apply([](const auto& x, const auto& y)
        {
            return evaluate_match(x, y);
        }, args_, std::forward_as_tuple(values_to_test...));
    }

    template <class... U>
    constexpr decltype(auto) get(const U&... values_to_test) &
    {
        if constexpr (std::is_invocable_v<T>)
            return result_();

        else if constexpr (std::is_invocable_v<T, U...>)
            return result_(values_to_test...);

        else
            return result_;
    }

    template <class... U>
    constexpr decltype(auto) get(const U&... values_to_test) &&
    {
        if constexpr (std::is_invocable_v<T>)
            return result_();

        else if constexpr (std::is_invocable_v<T, U...>)
            return result_(values_to_test...);

        else
            return std::move(result_);
    }

    template <class... U>
    constexpr decltype(auto) get(const U&... values_to_test) const &
    {
        if constexpr (std::is_invocable_v<const T>)
            return result_();

        else if constexpr (std::is_invocable_v<const T, U...>)
            return result_(values_to_test...);

        else
            return result_;
    }

    template <class... U>
    constexpr decltype(auto) get(const U&... values_to_test) const &&
    {
        if constexpr (std::is_invocable_v<const T>)
            return result_();

        else if constexpr (std::is_invocable_v<const T, U...>)
            return result_(values_to_test...);

        else
            return std::move(result_);
    }

private:
    [[no_unique_address]] T result_;
    [[no_unique_address]] std::tuple<Args...> args_;
};

} // namespace ptm
