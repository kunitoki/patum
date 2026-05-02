/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2026 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

// clang-format off

#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <re2/re2.h>
#include <regex>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <variant>


// Begin File: include/patum/concepts.h

namespace ptm {

template <class T>
concept StringLike = std::is_convertible_v<T, std::string_view>;

template <class T>
concept boolean_testable = requires(T&& v)
    {
        { static_cast<bool>(v) } -> std::convertible_to<bool>;
    };

template <class T>
concept dereferenceable = requires(T&& v)
    {
        { *v };
    };

template <class T, class U>
concept dereferenceable_comparable_with = requires(T&& v)
    {
        { *v } -> std::equality_comparable_with<U>;
    };

template <class U, class... Ts>
concept all_equality_comparable_with = requires(Ts... values)
    {
        { std::conditional_t<(std::equality_comparable_with<U, decltype(values)> && ...),
            std::true_type,
            std::false_type>{} } -> std::same_as<std::true_type>;
    };

} 


// End File: include/patum/concepts.h

// Begin File: include/patum/expect.h

namespace ptm {

constexpr void expect(bool condition)
{
    if (not condition)
    {
        assert(condition);
    }
}

} 


// End File: include/patum/expect.h

// Begin File: include/patum/features.h

#if __has_include(<re2/re2.h>)
#define PATUM_HAS_FEATURE_RE2 1
#else
#define PATUM_HAS_FEATURE_RE2 0
#endif


// End File: include/patum/features.h

// Begin File: include/patum/type_traits.h

namespace ptm {

template <class... T>
inline static constexpr bool always_false_v = false;

template <class T, template <class...> class Primary>
struct is_specialization_of;

template <class T, template <class...> class Primary>
struct is_specialization_of : std::false_type
{
};

template <template<class...> class Primary, class... Args>
struct is_specialization_of<Primary<Args...>, Primary> : std::true_type
{
};

template <class T, template <class...> class Primary>
inline static constexpr bool is_specialization_of_v = is_specialization_of<T, Primary>::value;

template <class T>
struct is_variant : std::false_type
{
};

template <class... Ts>
struct is_variant<std::variant<Ts...>> : std::true_type
{
};

template <class T>
inline static constexpr bool is_variant_v = is_variant<T>::value;

template <class T, class U>
struct is_variant_holding;

template <class T, class... Ts>
struct is_variant_holding<T, std::variant<Ts...>> : std::bool_constant<(std::is_same_v<T, Ts> || ...)>
{
};

template <class T, class U>
inline static constexpr bool is_variant_holding_v = is_variant_holding<T, U>::value;

template <class T, class... Ts>
struct common_type_or
{
    template <class... Us>
    static std::common_type_t<Us...> test(decltype(void(std::common_type_t<Us...>()))*);

    template <class... Us>
    static T test(...);

    using type = decltype(test<Ts...>(nullptr));
};

template <class T, class... Ts>
using common_type_or_t = typename common_type_or<T, Ts...>::type;

struct not_found_t {};

template <class... Ts>
struct type_list {};

template <class List, class T>
struct append_type;

template <class... Ts, class T>
struct append_type<type_list<Ts...>, T>
{
    using type = type_list<Ts..., T>;
};

template <class List>
struct common_type
{
    using type = not_found_t;
};

template <>
struct common_type<type_list<>>
{
    using type = not_found_t;
};

template <class... Ts>
struct common_type<type_list<Ts...>>
{
    using type = common_type_or_t<not_found_t, Ts...>;
};

template <class... Ts>
using common_type_t = typename common_type<type_list<Ts...>>::type;

template <class List>
struct non_void_common_type;

template <>
struct non_void_common_type<type_list<>>
{
    using type = type_list<>;
};

template <class T, class... Ts>
struct non_void_common_type<type_list<T, Ts...>>
{
    using type = std::conditional_t<
        std::is_same_v<T, void>,
        typename non_void_common_type<type_list<Ts...>>::type,
        typename append_type<typename non_void_common_type<type_list<Ts...>>::type, T>::type>;
};

template <class... Ts>
using non_void_common_type_t = typename common_type<typename non_void_common_type<type_list<Ts...>>::type>::type;

} 


// End File: include/patum/type_traits.h

// Begin File: include/patum/tuple.h

namespace ptm {

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

} 


// End File: include/patum/tuple.h

// Begin File: include/patum/struct.h

namespace ptm {

struct any_type
{
    template <class T> operator T();
};

template <class T, std::size_t I>
using always_t = T;

template <class T, class ... Args>
auto is_aggregate_constructible_impl(int) -> decltype(T{ std::declval<Args>()... }, void(), std::true_type{});

template <class T, class ... Args>
auto is_aggregate_constructible_impl(...) -> std::false_type;

template <class T, class ... Args>
using is_aggregate_constructible = decltype(is_aggregate_constructible_impl<T, Args...>(0));

template <class T, class Seq> struct has_n_member_impl;

template <class T, std::size_t ... Is>
struct has_n_member_impl<T, std::index_sequence<Is...>> : is_aggregate_constructible<T, always_t<any_type, Is>...>
{
};

template <class T, std::size_t N>
using has_n_member = has_n_member_impl<T, std::make_index_sequence<N>>;

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

} 


// End File: include/patum/struct.h

// Begin File: include/patum/overload_set.h

namespace ptm {

template <class... L>
struct overload_set : L...
{
    using L::operator()...;

    constexpr overload_set(L... lambda)
        : L(std::move(lambda))...
    {
    }
};

} 


// End File: include/patum/overload_set.h

// Begin File: include/patum/predicate.h

namespace ptm {

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

inline static constexpr auto _u = predicate([](const auto& u) { return u; });
inline static constexpr auto _v = predicate([](const auto& v) { return v; });
inline static constexpr auto _w = predicate([](const auto& w) { return w; });
inline static constexpr auto _x = predicate([](const auto& x) { return x; });
inline static constexpr auto _y = predicate([](const auto& y) { return y; });
inline static constexpr auto _z = predicate([](const auto& z) { return z; });

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

template <class F>
constexpr auto operator!(const predicate<F>& m)
{
    return predicate([m](const auto& x) { return !m(x); });
}

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

template <class F>
constexpr auto operator~(const predicate<F>& m)
{
    return predicate([m](const auto& x) { return ~m(x); });
}

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

template <class... Args>
constexpr auto in(Args&&... values) noexcept
{
    return predicate([&values...]<class U>(const U& value_to_test)
        requires all_equality_comparable_with<U, Args...>
    {
        return (false || ... || evaluate_match(std::forward<Args>(values), value_to_test));
    });
}

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

template <class T, class Proj = std::identity>
constexpr auto find(T&& value, Proj proj = {}) noexcept
{
    return predicate([value = std::forward<T>(value), proj = std::forward<Proj>(proj)]<class U>(const U& value_to_test)
        requires std::ranges::input_range<U>
    {
        return std::ranges::find(value_to_test, std::move(value), std::move(proj));
    });
}

template <StringLike T>
auto sregex(T&& r) noexcept
{
    return predicate([r = std::regex(r)]<class U>(const U& value_to_test) 
        requires StringLike<U>
    {
        std::smatch base_match;
        if constexpr (not std::same_as<U, std::string>)
        {
            const std::string value = value_to_test; 
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

} 


// End File: include/patum/predicate.h

// Begin File: include/patum/wildcard.h

namespace ptm {

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

inline static constexpr auto _ = wildcard([]<class U>([[maybe_unused]] const U& value_to_test)
{
    return true;
});

} 


// End File: include/patum/wildcard.h

// Begin File: include/patum/matcher.h

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
} 

template <class F, class... U>
inline constexpr bool polymorphically_invocable_v = detail::polymorphically_invocable<F, U...>();

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

} 


// End File: include/patum/matcher.h

// Begin File: include/patum/pattern.h

namespace ptm {

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

} 


// End File: include/patum/pattern.h

// Begin File: include/patum/match_expression.h

namespace ptm {

template <class E>
struct match_expression
{
    constexpr explicit match_expression(const E& expression)
        : expression_(expression)
    {
    }

    template <class T> requires StringLike<T>
    constexpr explicit match_expression(const T& expression)
        : expression_(std::string_view(expression))
    {
    }

    constexpr decltype(auto) get() const
    {
        return expression_;
    }

private:
    std::conditional_t<
        StringLike<E>,
        std::string_view,
        const E&> expression_;
};

} 


// End File: include/patum/match_expression.h

// Begin File: include/patum/match.h

namespace ptm {

template <class... M>
constexpr bool compatible_patterns() noexcept
{
    return (true && ... && (requires { M::capture_count; }));
}

template <class... M>
constexpr bool compatible_patterns_args(std::size_t count) noexcept
{
    return (true && ... && (count == M::capture_count));
}

template <class M, class E>
constexpr auto match_expressions(const M& matcher, const E& expressions)
{
    return std::apply([&](const auto&... ex)
    {
        return matcher.check(ex...);
    }, expressions);
}

template <class M, class E>
constexpr auto test_expressions(const M& matcher, E&& expressions)
{
    return std::apply([&]<class... E2>(E2&&... ex)
    {
        return matcher.get(std::forward<E2>(ex)...);
    }, std::forward<E>(expressions));
}

template <class R, class M, class E>
constexpr void invoke_result_expressions(std::optional<R>& result, M&& matcher, E&& expressions)
{
    std::apply([&, m = std::forward<M>(matcher)]<class... E2>(E2&&... ex) mutable
    {
        if constexpr (std::same_as<decltype(m.get(std::forward<E2>(ex)...)), void>)
            std::move(m).get(std::forward<E2>(ex)...);
        else
            result.emplace(std::move(m).get(std::forward<E2>(ex)...));
    }, std::forward<E>(expressions));
}

template <class M, class E>
constexpr void invoke_expressions(M&& matcher, E&& expressions)
{
    std::apply([m = std::forward<M>(matcher)]<class... E2>(E2&&... ex) mutable
    {
        std::move(m).get(std::forward<E2>(ex)...);
    }, std::forward<E>(expressions));
}

template <class... E>
struct match_helper
{
    constexpr explicit match_helper(E&&... expressions)
        : expressions_(std::forward_as_tuple(expressions...))
    {
    }

    template <class... M>
        requires(compatible_patterns<M...>()
            && compatible_patterns_args<M...>(sizeof...(E))
            && sizeof...(M) != 0)
    constexpr auto operator()(M&&... matchers) const
    {
        using ReturnType = non_void_common_type_t<decltype(test_expressions(matchers, expressions_))...>;

        if constexpr (std::same_as<ReturnType, not_found_t>)
        {
            [[maybe_unused]] auto ignore = ((match_expressions(matchers, expressions_)
                && (void(invoke_expressions(std::forward<M>(matchers), expressions_)), 1)) || ...);
        }
        else
        {
            std::optional<ReturnType> result;

            [[maybe_unused]] auto ignore = ((match_expressions(matchers, expressions_)
                && (void(invoke_result_expressions(result, std::forward<M>(matchers), expressions_)), 1)) || ...);

            return result;
        }
    }

private:
    std::tuple<E...> expressions_;
};

template <class... Args>
[[nodiscard]] constexpr match_helper<Args...> match(Args&&... args)
{
    return match_helper<Args...>{ std::forward<Args>(args)... };
}

} 


// End File: include/patum/match.h

// Begin File: include/patum.h


// End File: include/patum.h

// Begin File: include/patum/func_traits.h

namespace ptm {
namespace detail {

template <bool IsMember, bool IsConst, class R, class... Args>
struct function_traits_base
{
    using result_type = R;

    using argument_types = std::tuple<Args...>;

    static constexpr auto arity = sizeof...(Args);

    static constexpr auto is_member = IsMember;

    static constexpr auto is_const = IsConst;
};

template <class, bool Enable>
struct function_traits_impl;

template <class R, class... Args>
struct function_traits_impl<R(Args...), true> : function_traits_base<false, false, R, Args...>
{
};

template <class R, class... Args>
struct function_traits_impl<R (&)(Args...), true> : function_traits_base<false, false, R, Args...>
{
};

template <class R, class... Args>
struct function_traits_impl<R (*)(Args...), true> : function_traits_base<false, false, R, Args...>
{
};

template <class C, class R, class... Args>
struct function_traits_impl<R (C::*)(Args...), true> : function_traits_base<true, false, R, Args...>
{
};

template <class C, class R, class... Args>
struct function_traits_impl<R (C::*)(Args...) const, true> : function_traits_base<true, true, R, Args...>
{
};

template <class R, class... Args>
struct function_traits_impl<R(Args...) noexcept, true> : function_traits_base<false, false, R, Args...>
{
};

template <class R, class... Args>
struct function_traits_impl<R (*)(Args...) noexcept, true> : function_traits_base<false, false, R, Args...>
{
};

template <class C, class R, class... Args>
struct function_traits_impl<R (C::*)(Args...) noexcept, true> : function_traits_base<true, false, R, Args...>
{
};

template <class C, class R, class... Args>
struct function_traits_impl<R (C::*)(Args...) const noexcept, true> : function_traits_base<true, true, R, Args...>
{
};

template <class F>
struct functor_traits_impl : function_traits_impl<decltype(&std::remove_cvref_t<F>::operator()), true>
{
};

template <class F>
concept is_functor = requires
{
    &std::remove_cvref_t<F>::operator();
};

} 

template <class F>
struct function_traits : std::conditional_t<detail::is_functor<F>,
                                            detail::functor_traits_impl<std::remove_cvref_t<F>>,
                                            detail::function_traits_impl<std::remove_cvref_t<F>, true>>
{
};

template <std::size_t I, class F, class = void>
struct function_argument_or_void
{
    using type = void;
};

template <std::size_t I, class F>
struct function_argument_or_void<I, F, std::enable_if_t<I < std::tuple_size_v<typename function_traits<F>::argument_types>>>
{
    using type = std::tuple_element_t<I, typename function_traits<F>::argument_types>;
};

template <std::size_t I, class F>
using function_argument_or_void_t = typename function_argument_or_void<I, F>::type;

template <class F>
using function_result_t = typename function_traits<F>::result_type;

template <std::size_t I, class F>
using function_argument_t = std::tuple_element_t<I, typename function_traits<F>::argument_types>;

template <class F>
using function_arguments_t = typename function_traits<F>::argument_types;

template <class F>
static constexpr std::size_t function_arity_v = function_traits<F>::arity;

template <class F>
static constexpr bool function_is_member_v = function_traits<F>::is_member;

template <class F>
static constexpr bool function_is_const_v = function_traits<F>::is_const;

template <class T, class = void>
struct is_callable
{
    static constexpr bool value = false;
};

template <class T>
struct is_callable<T, std::void_t<decltype(&std::remove_cvref_t<T>::operator())>>
{
    static constexpr bool value = true;
};

template <class T>
struct is_callable<T, std::enable_if_t<std::is_pointer_v<T> && std::is_function_v<std::remove_pointer_t<T>>>>
{
    static constexpr bool value = true;
};

template <class T>
struct is_callable<T, std::enable_if_t<std::is_member_function_pointer_v<T>>>
{
    static constexpr bool value = true;
};

template <class T>
inline static constexpr bool is_callable_v = is_callable<T>::value;

} 


// End File: include/patum/func_traits.h

// Begin File: include/patum/reference.h

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

} 


// End File: include/patum/reference.h
// clang-format on
