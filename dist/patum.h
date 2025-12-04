/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

// clang-format off

#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <iterator>
#include <optional>
#include <ranges>
#include <re2/re2.h>
#include <regex>
#include <string_view>
#include <tuple>
#include <type_traits>
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
#endif

namespace ptm {

#if __has_include(<re2/re2.h>)
inline static constexpr bool has_feature_re2 = true;
#else
inline static constexpr bool has_feature_re2 = false;
#endif

}


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
struct common_type;

template <>
struct common_type<type_list<>>
{
    using type = not_found_t;
};

template <class... Ts>
struct common_type<type_list<Ts...>>
{
    using type = std::common_type_t<Ts...>;
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
    return std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
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
    if constexpr (has_feature_re2)
    {
        return predicate([r = std::string_view(r)]<class U>(const U& value_to_test)
            requires StringLike<U>
        {
            std::string_view str = value_to_test;
            return re2::RE2::FullMatch(re2::StringPiece(str.data(), str.size()), re2::RE2(r));
        });
    }
    else
    {
        return sregex(std::forward<T>(r));
    }
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
constexpr auto test_expressions(const M& matcher, const E& expressions)
{
    return std::apply([&](const auto&... ex)
    {
        return matcher.get(ex...);
    }, expressions);
}

template <class R, class M, class E>
constexpr void invoke_result_expressions(std::optional<R>& result, M&& matcher, const E& expressions)
{
    std::apply([&, m = std::forward<M>(matcher)](const auto&... ex) mutable
    {
        if constexpr (std::same_as<decltype(m.get(ex...)), void>)
            std::move(m).get(ex...);
        else
            result.emplace(std::move(m).get(ex...));
    }, expressions);
}

template <class M, class E>
constexpr void invoke_expressions(M&& matcher, const E& expressions)
{
    std::apply([m = std::forward<M>(matcher)](const auto&... ex) mutable
    {
        std::move(m).get(ex...);
    }, expressions);
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
// clang-format on
