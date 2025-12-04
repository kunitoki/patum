/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#pragma once

#include <type_traits>
#include <tuple>
#include <utility>

namespace ptm {

//=================================================================================================

template <std::size_t N>
struct tuple_unpacker
{
    template <class F, class... Args1, class... Args2>
    static constexpr bool apply(const F& func, std::tuple<Args1...> t1, std::tuple<Args2...> t2)
    {
        return func(std::get<N - 1>(t1), std::get<N - 1>(t2)) && tuple_unpacker<N - 1>::apply(func, t1, t2);
    }
};

template <>
struct tuple_unpacker<0>
{
    template <class F, class... Args1, class... Args2>
    static constexpr bool apply(const F& func, std::tuple<Args1...> t1, std::tuple<Args2...> t2)
    {
        return true;
    }
};

//=================================================================================================

template <std::size_t N>
struct to_tuple_t;

template <>
struct to_tuple_t<0>
{
    template <class S>
    constexpr auto operator()(S&& s) const requires std::is_aggregate_v<std::remove_cvref_t<S>>
    {
        return std::make_tuple();
    }
};

template <>
struct to_tuple_t<1>
{
    template <class S>
    constexpr auto operator()(S&& s) const requires std::is_aggregate_v<std::remove_cvref_t<S>>
    {
        auto [e0] = std::forward<S>(s);
        return std::make_tuple(std::move(e0));
    }
};

template <>
struct to_tuple_t<2>
{
    template <class S>
    constexpr auto operator()(S&& s) const requires std::is_aggregate_v<std::remove_cvref_t<S>>
    {
        auto [e0, e1] = std::forward<S>(s);
        return std::make_tuple(std::move(e0), std::move(e1));
    }
};

template <>
struct to_tuple_t<3>
{
    template <class S>
    constexpr auto operator()(S&& s) const requires std::is_aggregate_v<std::remove_cvref_t<S>>
    {
        auto [e0, e1, e2] = std::forward<S>(s);
        return std::make_tuple(std::move(e0), std::move(e1), std::move(e2));
    }
};

template <>
struct to_tuple_t<4>
{
    template <class S>
    constexpr auto operator()(S&& s) const requires std::is_aggregate_v<std::remove_cvref_t<S>>
    {
        auto [e0, e1, e2, e3] = std::forward<S>(s);
        return std::make_tuple(std::move(e0), std::move(e1), std::move(e2), std::move(e3));
    }
};

template <>
struct to_tuple_t<5>
{
    template <class S>
    constexpr auto operator()(S&& s) const requires std::is_aggregate_v<std::remove_cvref_t<S>>
    {
        auto [e0, e1, e2, e3, e4] = std::forward<S>(s);
        return std::make_tuple(std::move(e0), std::move(e1), std::move(e2), std::move(e3), std::move(e4));
    }
};

template <>
struct to_tuple_t<6>
{
    template <class S>
    constexpr auto operator()(S&& s) const requires std::is_aggregate_v<std::remove_cvref_t<S>>
    {
        auto [e0, e1, e2, e3, e4, e5] = std::forward<S>(s);
        return std::make_tuple(std::move(e0), std::move(e1), std::move(e2), std::move(e3), std::move(e4), std::move(e5));
    }
};

template <>
struct to_tuple_t<7>
{
    template <class S>
    constexpr auto operator()(S&& s) const requires std::is_aggregate_v<std::remove_cvref_t<S>>
    {
        auto [e0, e1, e2, e3, e4, e5, e6] = std::forward<S>(s);
        return std::make_tuple(std::move(e0), std::move(e1), std::move(e2), std::move(e3), std::move(e4), std::move(e5), std::move(e6));
    }
};

template <>
struct to_tuple_t<8>
{
    template <class S>
    constexpr auto operator()(S&& s) const requires std::is_aggregate_v<std::remove_cvref_t<S>>
    {
        auto [e0, e1, e2, e3, e4, e5, e6, e7] = std::forward<S>(s);
        return std::make_tuple(std::move(e0), std::move(e1), std::move(e2), std::move(e3), std::move(e4), std::move(e5), std::move(e6), std::move(e7));
    }
};

} // namespace ptm
