/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#pragma once

#include <concepts>
#include <iterator>
#include <ranges>
#include <regex>
#include <tuple>
#include <utility>
#include <variant>

#include "concepts.h"
#include "features.h"
#include "type_traits.h"
#include "tuple.h"

namespace ptm {

//=================================================================================================

template <class T, class U>
constexpr bool evaluate_match(const T& lhs, const U& rhs)
{
    if constexpr (requires { { lhs(rhs) } -> std::convertible_to<bool>; })
        return lhs(rhs);

    else if constexpr (StringLike<std::remove_cvref_t<T>> and StringLike<std::remove_cvref_t<U>>)
        return std::string_view(lhs) == std::string_view(rhs);

    else
        return lhs == rhs;
}

//=================================================================================================

template <class F>
struct predicate
{
    constexpr predicate(F&& func)
        : func_(std::move(func))
    {
    }

    constexpr auto operator()(const auto& v) const
    {
        return func_(v);
    }

protected:
    [[no_unique_address]] F func_;
};

//=================================================================================================

template <class T>
struct is_predicate : std::false_type
{
};

template <class F>
struct is_predicate<predicate<F>> : std::true_type
{
};

template <class T>
inline static constexpr bool is_predicate_v = is_predicate<T>::value;

//=================================================================================================

inline static constexpr auto _u = predicate([](const auto& u) { return u; });
inline static constexpr auto _v = predicate([](const auto& v) { return v; });
inline static constexpr auto _w = predicate([](const auto& w) { return w; });
inline static constexpr auto _x = predicate([](const auto& x) { return x; });
inline static constexpr auto _y = predicate([](const auto& y) { return y; });
inline static constexpr auto _z = predicate([](const auto& z) { return z; });

//=================================================================================================

template <class F, class T>
constexpr auto operator==(const predicate<F>& m, const T& t)
{
    return predicate([m, &t](const auto& x) { return m(x) == t; });
}

template <class F, class T>
constexpr auto operator==(const T& t, const predicate<F>& m)
{
    return predicate([m, &t](const auto& x) { return t == m(x); });
}

template <class F, class F2>
constexpr auto operator==(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) == m2(x); });
}

//=================================================================================================

template <class F, class T>
constexpr auto operator!=(const predicate<F>& m, const T& t)
{
    return predicate([m, &t](const auto& x) { return m(x) != t; });
}

template <class F, class T>
constexpr auto operator!=(const T& t, const predicate<F>& m)
{
    return predicate([m, &t](const auto& x) { return t != m(x); });
}

template <class F, class F2>
constexpr auto operator!=(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) != m2(x); });
}

//=================================================================================================

template <class F, class T>
constexpr auto operator<=(const predicate<F>& m, const T& t)
{
    return predicate([m, &t](const auto& x) { return m(x) <= t; });
}

template <class F, class T>
constexpr auto operator<=(const T& t, const predicate<F>& m)
{
    return predicate([m, &t](const auto& x) { return t <= m(x); });
}

template <class F, class F2>
constexpr auto operator<=(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) <= m2(x); });
}

//=================================================================================================

template <class F, class T>
constexpr auto operator>=(const predicate<F>& m, const T& t)
{
    return predicate([m, &t](const auto& x) { return m(x) >= t; });
}

template <class F, class T>
constexpr auto operator>=(const T& t, const predicate<F>& m)
{
    return predicate([m, &t](const auto& x) { return t >= m(x); });
}

template <class F, class F2>
constexpr auto operator>=(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) >= m2(x); });
}

//=================================================================================================

template <class F, class T>
constexpr auto operator<(const predicate<F>& m, const T& t)
{
    return predicate([m, &t](const auto& x) { return m(x) < t; });
}

template <class F, class T>
constexpr auto operator<(const T& t, const predicate<F>& m)
{
    return predicate([m, &t](const auto& x) { return t < m(x); });
}

template <class F, class F2>
constexpr auto operator<(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) < m2(x); });
}

//=================================================================================================

template <class F, class T>
constexpr auto operator>(const predicate<F>& m, const T& t)
{
    return predicate([m, &t](const auto& x) { return m(x) > t; });
}

template<class F, class T>
constexpr auto operator>(const T& t, const predicate<F>& m)
{
    return predicate([m, &t](const auto& x) { return t > m(x); });
}

template <class F, class F2>
constexpr auto operator>(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) > m2(x); });
}

//=================================================================================================

template <class F>
constexpr auto operator!(const predicate<F>& m)
{
    return predicate([m](const auto& x) { return !m(x); });
}

//=================================================================================================

template <class F, class F2>
constexpr auto operator&&(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) && m2(x); });
}

template <class F, class F2>
constexpr auto operator||(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) || m2(x); });
}

//=================================================================================================

template <class F>
constexpr auto operator+(const predicate<F>& m)
{
    return predicate([m](const auto& x) { return +m(x); });
}

template <class F, class T>
constexpr auto operator+(const predicate<F>& m, const T& t)
{
    return predicate([m, &t](const auto& x) { return m(x) + t; });
}

template<class F, class T>
constexpr auto operator+(const T& t, const predicate<F>& m)
{
    return predicate([m, &t](const auto& x) { return t + m(x); });
}

template <class F, class F2>
constexpr auto operator+(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) + m2(x); });
}

//=================================================================================================

template <class F>
constexpr auto operator-(const predicate<F>& m)
{
    return predicate([m](const auto& x) { return -m(x); });
}

template <class F, class T>
constexpr auto operator-(const predicate<F>& m, const T& t)
{
    return predicate([m, &t](const auto& x) { return m(x) - t; });
}

template<class F, class T>
constexpr auto operator-(const T& t, const predicate<F>& m)
{
    return predicate([m, &t](const auto& x) { return t - m(x); });
}

template <class F, class F2>
constexpr auto operator-(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) - m2(x); });
}

//=================================================================================================

template <class F, class T>
constexpr auto operator*(const predicate<F>& m, const T& t)
{
    return predicate([m, &t](const auto& x) { return m(x) * t; });
}

template<class F, class T>
constexpr auto operator*(const T& t, const predicate<F>& m)
{
    return predicate([m, &t](const auto& x) { return t * m(x); });
}

template <class F, class F2>
constexpr auto operator*(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) * m2(x); });
}

//=================================================================================================

template <class F, class T>
constexpr auto operator/(const predicate<F>& m, const T& t)
{
    return predicate([m, &t](const auto& x) { return m(x) / t; });
}

template<class F, class T>
constexpr auto operator/(const T& t, const predicate<F>& m)
{
    return predicate([m, &t](const auto& x) { return t / m(x); });
}

template <class F, class F2>
constexpr auto operator/(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) / m2(x); });
}

//=================================================================================================

template <class F>
constexpr auto operator~(const predicate<F>& m)
{
    return predicate([m](const auto& x) { return ~m(x); });
}

//=================================================================================================

template <class F, class T>
constexpr auto operator%(const predicate<F>& m, const T& t)
{
    return predicate([m, &t](const auto& x) { return m(x) % t; });
}

template <class F, class T>
constexpr auto operator%(const T& t, const predicate<F>& m)
{
    return predicate([m, &t](const auto& x) { return t % m(x); });
}

template <class F, class F2>
constexpr auto operator%(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) % m2(x); });
}

//=================================================================================================

template <class F, class T>
constexpr auto operator&(const predicate<F>& m, const T& t)
{
    return predicate([m, &t](const auto& x) { return m(x) & t; });
}

template <class F, class T>
constexpr auto operator&(const T& t, const predicate<F>& m)
{
    return predicate([m, &t](const auto& x) { return t & m(x); });
}

template <class F, class F2>
constexpr auto operator&(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) & m2(x); });
}

//=================================================================================================

template <class F, class T>
constexpr auto operator|(const predicate<F>& m, const T& t)
{
    return predicate([m, &t](const auto& x) { return m(x) | t; });
}

template <class F, class T>
constexpr auto operator|(const T& t, const predicate<F>& m)
{
    return predicate([m, &t](const auto& x) { return t | m(x); });
}

template <class F, class F2>
constexpr auto operator|(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) | m2(x); });
}

//=================================================================================================

template <class F, class T>
constexpr auto operator^(const predicate<F>& m, const T& t)
{
    return predicate([m, &t](const auto& x) { return m(x) ^ t; });
}

template <class F, class T>
constexpr auto operator^(const T& t, const predicate<F>& m)
{
    return predicate([m, &t](const auto& x) { return t ^ m(x); });
}

template <class F, class F2>
constexpr auto operator^(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) ^ m2(x); });
}

//=================================================================================================

template <class F, class T>
constexpr auto operator>>(const predicate<F>& m, const T& t)
{
    return predicate([m, &t](const auto& x) { return m(x) >> t; });
}

template <class F, class T>
constexpr auto operator>>(const T& t, const predicate<F>& m)
{
    return predicate([m, &t](const auto& x) { return t << m(x); });
}

template <class F, class F2>
constexpr auto operator>>(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) >> m2(x); });
}

//=================================================================================================

template <class F, class T>
constexpr auto operator<<(const predicate<F>& m, const T& t)
{
    return predicate([m, &t](const auto& x) { return m(x) >> t; });
}

template <class F, class T>
constexpr auto operator<<(const T& t, const predicate<F>& m)
{
    return predicate([m, &t](const auto& x) { return t << m(x); });
}

template <class F, class F2>
constexpr auto operator<<(const predicate<F>& m, const predicate<F2>& m2)
{
    return predicate([m, m2](const auto& x) { return m(x) << m2(x); });
}

//=================================================================================================

template <std::integral T>
constexpr auto range(T first, T last) noexcept
{
    expect(first < last);

    return predicate([first, last]<class U>(const U& value_to_test)
        requires std::equality_comparable_with<T, U>
    {
        return value_to_test >= first and value_to_test <= last;
    });
}

//=================================================================================================

template <class... Args>
constexpr auto in(Args&&... values) noexcept
{
    return predicate([&values...]<class U>(const U& value_to_test)
        requires all_equality_comparable_with<U, Args...>
    {
        return (false || ... || evaluate_match(std::forward<Args>(values), value_to_test));
    });
}

//=================================================================================================

template <class T>
constexpr auto some(T&& value) noexcept
{
    return predicate([value = std::forward<T>(value)]<class U>(const U& value_to_test)
        requires boolean_testable<U> && dereferenceable_comparable_with<U, T>
    {
        return static_cast<bool>(value_to_test) and evaluate_match(value, *value_to_test);
    });
}

constexpr auto some() noexcept
{
    return predicate([]<class U>(const U& value_to_test)
        requires boolean_testable<U>
    {
        return static_cast<bool>(value_to_test);
    });
}

inline static constexpr auto none = predicate([]<class U>(const U& value_to_test)
    requires boolean_testable<U>
{
    return not static_cast<bool>(value_to_test);
});

//=================================================================================================

template <class T>
constexpr auto valued(T&& value) noexcept
{
    return predicate([value = std::forward<T>(value)]<class U>(const U& value_to_test)
        requires std::equality_comparable<T> and requires(const U u)
            {
                { std::holds_alternative<T>(u) } -> std::convertible_to<bool>;
                { std::get<T>(u) } -> std::same_as<const T&>;
            }
    {
        return std::holds_alternative<T>(value_to_test) && evaluate_match(value, std::get<T>(value_to_test));
    });
}

template <class T>
inline static constexpr auto typed = predicate([]<class U>([[maybe_unused]] const U& value_to_test)
    requires is_variant_v<U> and requires{ is_variant_holding<T, U>(); }
{
    return is_variant_holding<T, U>();
});

//=================================================================================================

template <class T>
inline static constexpr auto is = predicate([]<class U>([[maybe_unused]] const U& value_to_test)
{
    return std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
});

//=================================================================================================

template <class... Args>
constexpr auto ds(Args&&... values) noexcept
{
    using T = std::tuple<Args...>;

    return predicate([value = std::forward_as_tuple(values...)]<class U>(const U& value_to_test)
    {
        if constexpr (std::is_aggregate_v<U> && member_count_v<U> >= sizeof...(Args))
        {
            return tuple_unpacker<sizeof...(Args)>::apply([](const auto& x, const auto& y)
                requires requires{ { evaluate_match(x, y) } -> std::same_as<bool>; }
            {
                return evaluate_match(x, y);
            }, value, to_tuple_t<member_count_v<U>>{}(value_to_test));
        }
        else if constexpr (std::tuple_size_v<U> >= sizeof...(Args))
        {
            return tuple_unpacker<sizeof...(Args)>::apply([](const auto& x, const auto& y)
                requires requires{ { evaluate_match(x, y) } -> std::same_as<bool>; }
            {
                return evaluate_match(x, y);
            }, value, value_to_test);
        }
        else
        {
            static_assert(always_false_v<T, U>, "Impossible to evaluate destructure matching between T and U");
        }
    });
}

//=================================================================================================

constexpr auto sized(std::size_t count) noexcept
{
    return predicate([count]<class U>(const U& value_to_test)
        requires requires(const U u) { { std::size(u) } -> std::equality_comparable_with<std::size_t>; }
    {
        return std::size(value_to_test) == count;
    });
}

template <class F>
constexpr auto size(const predicate<F>& m) noexcept
{
    return predicate([m]<class U>(const U& value_to_test)
        requires requires(const U u) { { std::size(m(u)) } -> std::equality_comparable_with<std::size_t>; }
    {
        return std::size(m(value_to_test));
    });
}

//=================================================================================================

constexpr auto ssized(std::ptrdiff_t count) noexcept
{
    return predicate([count]<class U>(const U& value_to_test)
        requires requires(const U u) { { std::ssize(u) } -> std::equality_comparable_with<std::ptrdiff_t>; }
    {
        return std::ssize(value_to_test) == count;
    });
}

template <class F>
constexpr auto ssize(const predicate<F>& m) noexcept
{
    return predicate([m]<class U>(const U& value_to_test)
        requires requires(const U u) { { std::ssize(m(u)) } -> std::equality_comparable_with<std::ptrdiff_t>; }
    {
        return std::ssize(m(value_to_test));
    });
}

//=================================================================================================

constexpr auto begin() noexcept
{
    return predicate([]<class U>(const U& value_to_test)
        requires std::ranges::input_range<U>
    {
        return std::ranges::begin(value_to_test);
    });
}

constexpr auto end() noexcept
{
    return predicate([]<class U>(const U& value_to_test)
        requires std::ranges::input_range<U>
    {
        return std::ranges::end(value_to_test);
    });
}

template <class F>
constexpr auto next(const predicate<F>& m, std::ptrdiff_t count = 1) noexcept
{
    return predicate([m, count]<class U>(const U& value_to_test)
        requires requires(const U u)
            {
                { std::ranges::next(m(u), count) } -> std::input_iterator;
            }
    {
        return std::ranges::next(m(value_to_test), count);
    });
}

template <class F>
constexpr auto prev(const predicate<F>& m, std::ptrdiff_t count = 1) noexcept
{
    return predicate([m, count]<class U>(const U& value_to_test)
        requires requires(const U u)
            {
                { std::ranges::prev(m(u), count) } -> std::bidirectional_iterator;
            }
    {
        return std::ranges::prev(m(value_to_test), count);
    });
}

//=================================================================================================

template <class T, class Proj = std::identity>
constexpr auto find(T&& value, Proj proj = {}) noexcept
{
    return predicate([value = std::forward<T>(value), proj = std::forward<Proj>(proj)]<class U>(const U& value_to_test)
        requires std::ranges::input_range<U>
    {
        return std::ranges::find(value_to_test, std::move(value), std::move(proj));
    });
}

//=================================================================================================

template <StringLike T>
auto sregex(T&& r) noexcept
{
    return predicate([r = std::regex(r)]<class U>(const U& value_to_test) // TODO - avoid costly instantiation
        requires StringLike<U>
    {
        std::smatch base_match;
        if constexpr (not std::same_as<U, std::string>)
        {
            const std::string value = value_to_test; // TODO - avoid copy if possible
            auto result = std::regex_match(value, base_match, r);
            return result;
        }
        else
        {
            auto result = std::regex_match(value_to_test, base_match, r);
            return result;
        }
    });
}

template <StringLike T>
auto regex(T&& r) noexcept
{
#if PATUM_HAS_FEATURE_RE2
    return predicate([r = std::string_view(r)]<class U>(const U& value_to_test)
        requires StringLike<U>
    {
        std::string_view str = value_to_test;
        return re2::RE2::FullMatch(re2::StringPiece(str.data(), str.size()), re2::RE2(r));
    });
#else
    return sregex(std::forward<T>(r));
#endif
}

} // namespace ptm
