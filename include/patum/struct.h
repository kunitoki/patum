/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#pragma once

#include <algorithm>
#include <type_traits>
#include <utility>

namespace ptm {

//=================================================================================================

struct any_type
{
    template <class T> operator T();
};

template <class T, std::size_t I>
using always_t = T;

//=================================================================================================

template <class T, class ... Args>
auto is_aggregate_constructible_impl(int) -> decltype(T{ std::declval<Args>()... }, void(), std::true_type{});

template <class T, class ... Args>
auto is_aggregate_constructible_impl(...) -> std::false_type;

template <class T, class ... Args>
using is_aggregate_constructible = decltype(is_aggregate_constructible_impl<T, Args...>(0));

//=================================================================================================

template <class T, class Seq> struct has_n_member_impl;

template <class T, std::size_t ... Is>
struct has_n_member_impl<T, std::index_sequence<Is...>> : is_aggregate_constructible<T, always_t<any_type, Is>...>
{
};

template <class T, std::size_t N>
using has_n_member = has_n_member_impl<T, std::make_index_sequence<N>>;

//=================================================================================================

template <class T, class Seq>
struct member_count_impl;

template <class T, std::size_t... Is>
struct member_count_impl<T, std::index_sequence<Is...>> : std::integral_constant<std::size_t, (std::max)({ (has_n_member<T, Is>() * Is)... })>
{
};

template <class T>
using member_count = member_count_impl<T, std::make_index_sequence<1 + sizeof (T)>>;

template <class T>
inline static constexpr auto member_count_v = member_count<T>::value;

} // namespace ptm
