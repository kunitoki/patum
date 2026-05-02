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
#include <memory>
#include <ranges>
#include <regex>
#include <typeinfo>
#include <tuple>
#include <utility>
#include <variant>

#include "concepts.h"
#include "features.h"
#include "type_traits.h"
#include "tuple.h"

namespace ptm {

//=================================================================================================

namespace detail {

template <class T, class U>
concept predicate_match_evaluable = requires(const T& lhs, const U& rhs)
{
    { lhs(rhs) } -> std::convertible_to<bool>;
};

template <class T, class U>
concept string_match_evaluable =
    StringLike<std::remove_cvref_t<T>> && StringLike<std::remove_cvref_t<U>>;

template <class T, class U>
concept equality_match_evaluable = requires(const T& lhs, const U& rhs)
{
    { lhs == rhs } -> std::convertible_to<bool>;
};

template <class T, class U>
inline static constexpr bool match_evaluable_v =
    predicate_match_evaluable<T, U> ||
    string_match_evaluable<T, U> ||
    equality_match_evaluable<T, U>;

}

//=================================================================================================

template <class T, class U>
constexpr bool evaluate_match(const T& lhs, const U& rhs)
{
    if constexpr (detail::predicate_match_evaluable<T, U>)
        return lhs(rhs);

    else if constexpr (detail::string_match_evaluable<T, U>)
        return std::string_view(lhs) == std::string_view(rhs);

    else if constexpr (detail::equality_match_evaluable<T, U>)
        return lhs == rhs;

    else
    {
        static_assert(
            always_false_v<T, U>,
            "Pattern cannot be evaluated against the matched value: use a predicate returning bool or provide operator=="
        );
    }
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
    return predicate([m, &t](const auto& x) { return m(x) << t; });
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
    using T2 = std::remove_cvref_t<T>;
    using U2 = std::remove_cvref_t<U>;

    if constexpr (std::same_as<T2, U2> || std::is_base_of_v<T2, U2>)
        return true;

    else if constexpr (std::is_pointer_v<U2> && std::is_class_v<T2>)
    {
        using From = std::remove_pointer_t<U2>;
        using From2 = std::remove_cv_t<From>;
        using ToPtr = std::conditional_t<std::is_const_v<From>, const T2*, T2*>;

        if constexpr (std::same_as<T2, From2> || std::is_base_of_v<T2, From2>)
            return true;

        else if constexpr (std::is_polymorphic_v<T2> && std::is_polymorphic_v<From2>)
            return dynamic_cast<ToPtr>(value_to_test) != nullptr;

        else
            return false;
    }

    else if constexpr (std::is_polymorphic_v<T2> && std::is_polymorphic_v<U2>)
        return dynamic_cast<const T2*>(std::addressof(value_to_test)) != nullptr;

    else
        return false;
});

//=================================================================================================

namespace detail {

template <class T>
concept tuple_like = requires
{
    typename std::tuple_size<std::remove_cvref_t<T>>::type;
};

template <class T>
inline static constexpr bool aggregate_like_v = std::is_aggregate_v<std::remove_cvref_t<T>>;

template <class T>
inline static constexpr std::size_t aggregate_size_v = []
{
    using T2 = std::remove_cvref_t<T>;

    if constexpr (aggregate_like_v<T2>)
        return member_count_v<T2>;
    else
        return std::size_t{ 0 };
}();

template <class T>
inline static constexpr std::size_t tuple_like_size_v = []
{
    using T2 = std::remove_cvref_t<T>;

    if constexpr (tuple_like<T2>)
        return std::tuple_size_v<T2>;
    else
        return std::size_t{ 0 };
}();

template <class T>
inline static constexpr bool destructurable_candidate_v = aggregate_like_v<T> || tuple_like<T>;

template <class T, std::size_t N>
inline static constexpr bool aggregate_destructurable_v = aggregate_size_v<T> >= N;

template <class T, std::size_t N>
inline static constexpr bool tuple_destructurable_v = tuple_like_size_v<T> >= N;

template <class T, std::size_t N>
inline static constexpr bool destructurable_v =
    aggregate_destructurable_v<T, N> || tuple_destructurable_v<T, N>;

template <std::size_t N>
struct forward_to_tuple_t;

template <>
struct forward_to_tuple_t<0>
{
    template <class S>
    constexpr auto operator()(S&&) const
    {
        return std::forward_as_tuple();
    }
};

template <>
struct forward_to_tuple_t<1>
{
    template <class S>
    constexpr auto operator()(S&& s) const
    {
        auto&& [e0] = std::forward<S>(s);
        return std::forward_as_tuple(e0);
    }
};

template <>
struct forward_to_tuple_t<2>
{
    template <class S>
    constexpr auto operator()(S&& s) const
    {
        auto&& [e0, e1] = std::forward<S>(s);
        return std::forward_as_tuple(e0, e1);
    }
};

template <>
struct forward_to_tuple_t<3>
{
    template <class S>
    constexpr auto operator()(S&& s) const
    {
        auto&& [e0, e1, e2] = std::forward<S>(s);
        return std::forward_as_tuple(e0, e1, e2);
    }
};

template <>
struct forward_to_tuple_t<4>
{
    template <class S>
    constexpr auto operator()(S&& s) const
    {
        auto&& [e0, e1, e2, e3] = std::forward<S>(s);
        return std::forward_as_tuple(e0, e1, e2, e3);
    }
};

template <>
struct forward_to_tuple_t<5>
{
    template <class S>
    constexpr auto operator()(S&& s) const
    {
        auto&& [e0, e1, e2, e3, e4] = std::forward<S>(s);
        return std::forward_as_tuple(e0, e1, e2, e3, e4);
    }
};

template <>
struct forward_to_tuple_t<6>
{
    template <class S>
    constexpr auto operator()(S&& s) const
    {
        auto&& [e0, e1, e2, e3, e4, e5] = std::forward<S>(s);
        return std::forward_as_tuple(e0, e1, e2, e3, e4, e5);
    }
};

template <>
struct forward_to_tuple_t<7>
{
    template <class S>
    constexpr auto operator()(S&& s) const
    {
        auto&& [e0, e1, e2, e3, e4, e5, e6] = std::forward<S>(s);
        return std::forward_as_tuple(e0, e1, e2, e3, e4, e5, e6);
    }
};

template <>
struct forward_to_tuple_t<8>
{
    template <class S>
    constexpr auto operator()(S&& s) const
    {
        auto&& [e0, e1, e2, e3, e4, e5, e6, e7] = std::forward<S>(s);
        return std::forward_as_tuple(e0, e1, e2, e3, e4, e5, e6, e7);
    }
};

template <std::size_t N, class T, std::size_t... I>
constexpr auto take_tuple(T&& value, std::index_sequence<I...>)
{
    return std::forward_as_tuple(std::get<I>(std::forward<T>(value))...);
}

template <std::size_t N, class T>
constexpr auto take_tuple(T&& value)
{
    return take_tuple<N>(std::forward<T>(value), std::make_index_sequence<N>{});
}

}

template <class... Args>
struct destructure_pattern
{
    using value_type = std::tuple<Args...>;

    destructure_pattern(const destructure_pattern&) = default;
    destructure_pattern(destructure_pattern&&) = default;

    template <class... U>
        requires(sizeof...(U) == sizeof...(Args))
    constexpr explicit destructure_pattern(U&&... values)
        : value_(std::forward<U>(values)...)
    {
    }

    template <class U>
    constexpr bool operator()(const U& value_to_test) const
    {
        using U2 = std::remove_cvref_t<U>;

        if constexpr (detail::aggregate_destructurable_v<U2, sizeof...(Args)>)
        {
            return tuple_unpacker<sizeof...(Args)>::apply([](const auto& x, const auto& y)
                requires requires{ { evaluate_match(x, y) } -> std::same_as<bool>; }
            {
                return evaluate_match(x, y);
            }, value_, to_tuple_t<member_count_v<U2>>{}(value_to_test));
        }
        else if constexpr (detail::tuple_destructurable_v<U2, sizeof...(Args)>)
        {
            return tuple_unpacker<sizeof...(Args)>::apply([](const auto& x, const auto& y)
                requires requires{ { evaluate_match(x, y) } -> std::same_as<bool>; }
            {
                return evaluate_match(x, y);
            }, value_, value_to_test);
        }
        else
        {
            if constexpr (detail::destructurable_candidate_v<U2>)
                static_assert(always_false_v<value_type, U>, "Destructure pattern has more fields than the matched value");
            else
                static_assert(always_false_v<value_type, U>, "Destructure pattern requires a tuple-like or aggregate matched value");
        }
    }

    template <class U>
        requires detail::destructurable_v<U, sizeof...(Args)>
    constexpr auto captures(U&& value_to_test) const
    {
        using U2 = std::remove_cvref_t<U>;

        if constexpr (detail::aggregate_destructurable_v<U2, sizeof...(Args)>)
        {
            auto members = detail::forward_to_tuple_t<member_count_v<U2>>{}(std::forward<U>(value_to_test));
            return detail::take_tuple<sizeof...(Args)>(members);
        }
        else if constexpr (detail::tuple_destructurable_v<U2, sizeof...(Args)>)
        {
            return detail::take_tuple<sizeof...(Args)>(std::forward<U>(value_to_test));
        }
        else
        {
            static_assert(always_false_v<value_type, U>, "Impossible to capture destructure matching between T and U");
        }
    }

private:
    [[no_unique_address]] value_type value_;
};

template <class... Args>
constexpr auto ds(Args&&... values) noexcept
{
    return destructure_pattern<std::decay_t<Args>...>{ std::forward<Args>(values)... };
}

template <class T, class U>
constexpr auto match_captures([[maybe_unused]] const T& pattern, U&& value)
{
    return std::forward_as_tuple(std::forward<U>(value));
}

template <class... Args, class U>
    requires detail::destructurable_v<U, sizeof...(Args)>
constexpr auto match_captures(const destructure_pattern<Args...>& pattern, U&& value)
{
    return pattern.captures(std::forward<U>(value));
}

template <class... Args, class U>
    requires (!detail::destructurable_v<U, sizeof...(Args)>)
constexpr auto match_captures([[maybe_unused]] const destructure_pattern<Args...>& pattern, U&& value)
{
    if constexpr (detail::destructurable_candidate_v<U>)
        static_assert(always_false_v<U>, "Destructure pattern has more fields than the matched value");
    else
        static_assert(always_false_v<U>, "Destructure pattern requires a tuple-like or aggregate matched value");

    return std::forward_as_tuple(std::forward<U>(value));
}

//=================================================================================================

namespace detail {

template <class U, class T, std::size_t... I>
constexpr bool sequence_match_impl(const U& value_to_test, const T& value, std::index_sequence<I...>)
{
    using RangeDifference = std::ranges::range_difference_t<U>;

    if (std::ranges::distance(value_to_test) != static_cast<RangeDifference>(sizeof...(I)))
        return false;

    auto it = std::ranges::begin(value_to_test);
    return (true && ... && (evaluate_match(std::get<I>(value), *it++)));
}

template <class U, class T, std::size_t... I>
constexpr bool starts_with_impl(const U& value_to_test, const T& value, std::index_sequence<I...>)
{
    if (std::ranges::distance(value_to_test) < static_cast<std::ranges::range_difference_t<U>>(sizeof...(I)))
        return false;

    auto it = std::ranges::begin(value_to_test);
    return (true && ... && (evaluate_match(std::get<I>(value), *it++)));
}

template <class U, class T, std::size_t... I>
constexpr bool ends_with_impl(const U& value_to_test, const T& value, std::index_sequence<I...>)
{
    using RangeDifference = std::ranges::range_difference_t<U>;
    constexpr auto pattern_size = static_cast<RangeDifference>(sizeof...(I));
    const auto value_size = std::ranges::distance(value_to_test);

    if (value_size < pattern_size)
        return false;

    auto it = std::ranges::next(std::ranges::begin(value_to_test), value_size - pattern_size);
    return (true && ... && (evaluate_match(std::get<I>(value), *it++)));
}

}

//=================================================================================================

template <class... Args>
constexpr auto seq(Args&&... values) noexcept
{
    return predicate([value = std::forward_as_tuple(values...)]<class U>(const U& value_to_test)
        requires std::ranges::forward_range<U>
    {
        return detail::sequence_match_impl(value_to_test, value, std::make_index_sequence<sizeof...(Args)>{});
    });
}

template <class... Args>
constexpr auto starts_with(Args&&... values) noexcept
{
    return predicate([value = std::forward_as_tuple(values...)]<class U>(const U& value_to_test)
        requires std::ranges::forward_range<U>
    {
        return detail::starts_with_impl(value_to_test, value, std::make_index_sequence<sizeof...(Args)>{});
    });
}

template <class... Args>
constexpr auto ends_with(Args&&... values) noexcept
{
    return predicate([value = std::forward_as_tuple(values...)]<class U>(const U& value_to_test)
        requires std::ranges::forward_range<U>
    {
        return detail::ends_with_impl(value_to_test, value, std::make_index_sequence<sizeof...(Args)>{});
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

template <class T, class Proj = std::identity>
constexpr auto contains(T&& value, Proj proj = {}) noexcept
{
    return predicate([value = std::forward<T>(value), proj = std::forward<Proj>(proj)]<class U>(const U& value_to_test)
        requires std::ranges::input_range<U>
    {
        return std::ranges::find(value_to_test, value, proj) != std::ranges::end(value_to_test);
    });
}

constexpr auto empty() noexcept
{
    return predicate([]<class U>(const U& value_to_test)
        requires std::ranges::range<U>
    {
        return std::ranges::empty(value_to_test);
    });
}

constexpr auto non_empty() noexcept
{
    return predicate([]<class U>(const U& value_to_test)
        requires std::ranges::range<U>
    {
        return not std::ranges::empty(value_to_test);
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
