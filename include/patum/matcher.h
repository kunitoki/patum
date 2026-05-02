/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#pragma once

#include <concepts>
#include <functional>
#include <type_traits>
#include <tuple>
#include <utility>

#include "type_traits.h"
#include "tuple.h"
#include "reference.h"
#include "func_traits.h"

namespace ptm {
namespace detail {

template <class T>
concept complete_class = std::is_class_v<T> && requires { sizeof(T); };

template <class T>
inline constexpr bool polymorphic_ok_v = complete_class<T> && std::is_polymorphic_v<T>;

template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <class To, class From>
inline constexpr bool can_polymorphic_cast_v = []
{
    using From0 = remove_cvref_t<From>;
    using To0   = remove_cvref_t<To>;

    if constexpr (std::is_pointer_v<From0> && std::is_pointer_v<To0>)
    {
        using F = std::remove_pointer_t<From0>;
        using T = std::remove_pointer_t<To0>;
        if constexpr (!polymorphic_ok_v<F> || !std::is_class_v<T>)
            return false;
        else
            return std::is_base_of_v<T, F> || std::is_base_of_v<F, T>;
    }
    else if constexpr (std::is_reference_v<From> && std::is_reference_v<To>)
    {
        using F = remove_cvref_t<From>;
        using T = remove_cvref_t<To>;
        if constexpr (!polymorphic_ok_v<F> || !std::is_class_v<T>)
            return false;
        else
            return std::is_base_of_v<T, F> || std::is_base_of_v<F, T>;
    }
    else
    {
        return false;
    }
}();

template <class F, class... U, std::size_t... I>
consteval bool polymorphically_invocable_impl(std::index_sequence<I...>)
{
    if constexpr (std::is_invocable_v<F, U...>)
        return true;
    else
        return (can_polymorphic_cast_v<function_argument_t<I, F>, U> && ...);
}

template <class F, class... U>
consteval bool polymorphically_invocable()
{
    if constexpr (!is_callable_v<F>)
        return false;
    else if constexpr (function_arity_v<F> != sizeof...(U))
        return false;
    else
        return polymorphically_invocable_impl<F, U...>(std::make_index_sequence<sizeof...(U)>{});
}

template <class To, class From>
decltype(auto) polymorphic_cast(From&& v)
{
    if constexpr (std::is_pointer_v<std::remove_reference_t<From>>)
        return dynamic_cast<To>(v);
    else
        return dynamic_cast<To>(v);
}

template <class F, class... U, std::size_t... I>
decltype(auto) invoke_with_polymorphic_cast(F&& f, std::index_sequence<I...>, U&&... u)
{
    return std::invoke(
        std::forward<F>(f),
        polymorphic_cast<function_argument_t<I, std::remove_reference_t<F>>>(std::forward<U>(u))...
    );
}
} // namespace detail

template <class F, class... U>
inline constexpr bool polymorphically_invocable_v = detail::polymorphically_invocable<F, U...>();

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
    constexpr decltype(auto) get(U&&... values_to_test) &
    {
        if constexpr (std::is_invocable_v<T>)
            return result_();

        else if constexpr (std::is_invocable_v<T, decltype(std::forward<U>(values_to_test))...>)
            return result_(std::forward<U>(values_to_test)...);

        else if constexpr (std::is_invocable_v<T, decltype(std::move(values_to_test))...>)
            return result_(std::move(values_to_test)...);

        else if constexpr (std::is_invocable_v<T, decltype(dereference(values_to_test))...>)
            return result_(dereference(values_to_test)...);

        else if constexpr (polymorphically_invocable_v<decltype(result_), decltype(std::forward<U>(values_to_test))...>)
            return detail::invoke_with_polymorphic_cast(
                result_,
                std::make_index_sequence<sizeof...(values_to_test)>{},
                std::forward<U>(values_to_test)...
            );

        else if constexpr (is_callable_v<decltype(result_)>)
            static_assert(always_false_v<decltype(result_)>);

        else
            return result_;
    }

    template <class... U>
    constexpr decltype(auto) get(U&&... values_to_test) &&
    {
        if constexpr (std::is_invocable_v<T>)
            return result_();

        else if constexpr (std::is_invocable_v<T, decltype(std::forward<U>(values_to_test))...>)
            return result_(std::forward<U>(values_to_test)...);

        else if constexpr (std::is_invocable_v<T, decltype(std::move(values_to_test))...>)
            return result_(std::move(values_to_test)...);

        else if constexpr (std::is_invocable_v<T, decltype(dereference(values_to_test))...>)
            return result_(dereference(values_to_test)...);

        else if constexpr (polymorphically_invocable_v<decltype(result_), decltype(std::forward<U>(values_to_test))...>)
            return detail::invoke_with_polymorphic_cast(
                std::move(result_),
                std::make_index_sequence<sizeof...(values_to_test)>{},
                std::forward<U>(values_to_test)...
            );

        else if constexpr (is_callable_v<decltype(result_)>)
            static_assert(always_false_v<decltype(result_)>);

        else
            return std::move(result_);
    }

    template <class... U>
    constexpr decltype(auto) get(U&&... values_to_test) const &
    {
        if constexpr (std::is_invocable_v<const T>)
            return result_();

        else if constexpr (std::is_invocable_v<const T, decltype(std::forward<U>(values_to_test))...>)
            return result_(std::forward<U>(values_to_test)...);

        else if constexpr (std::is_invocable_v<const T, decltype(std::move(values_to_test))...>)
            return result_(std::move(values_to_test)...);

        else if constexpr (std::is_invocable_v<const T, decltype(dereference(values_to_test))...>)
            return result_(dereference(values_to_test)...);

        else if constexpr (polymorphically_invocable_v<decltype(result_), decltype(std::forward<U>(values_to_test))...>)
            return detail::invoke_with_polymorphic_cast(
                result_,
                std::make_index_sequence<sizeof...(values_to_test)>{},
                std::forward<U>(values_to_test)...
            );

        else if constexpr (is_callable_v<decltype(result_)>)
            static_assert(always_false_v<decltype(result_)>);

        else
            return result_;
    }

    template <class... U>
    constexpr decltype(auto) get(U&&... values_to_test) const &&
    {
        if constexpr (std::is_invocable_v<const T>)
            return result_();

        else if constexpr (std::is_invocable_v<const T, decltype(std::forward<U>(values_to_test))...>)
            return result_(std::forward<U>(values_to_test)...);

        else if constexpr (std::is_invocable_v<const T, decltype(std::move(values_to_test))...>)
            return result_(std::move(values_to_test)...);

        else if constexpr (std::is_invocable_v<const T, decltype(dereference(values_to_test))...>)
            return result_(dereference(values_to_test)...);

        else if constexpr (polymorphically_invocable_v<decltype(result_), decltype(std::forward<U>(values_to_test))...>)
            return detail::invoke_with_polymorphic_cast(
                result_,
                std::make_index_sequence<sizeof...(values_to_test)>{},
                std::forward<U>(values_to_test)...
            );

        else if constexpr (is_callable_v<decltype(result_)>)
            static_assert(always_false_v<decltype(result_)>);

        else
            return std::move(result_);
    }

private:
    [[no_unique_address]] T result_;
    [[no_unique_address]] std::tuple<Args...> args_;
};

} // namespace ptm
