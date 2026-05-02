/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#pragma once

#include <type_traits>
#include <utility>
#include <variant>

namespace ptm {

//=================================================================================================

template <class... T>
inline static constexpr bool always_false_v = false;

//=================================================================================================

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

//=================================================================================================

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

//=================================================================================================

template <class T, class U>
struct is_variant_holding;

template <class T, class... Ts>
struct is_variant_holding<T, std::variant<Ts...>> : std::bool_constant<(std::is_same_v<T, Ts> || ...)>
{
};

template <class T, class U>
inline static constexpr bool is_variant_holding_v = is_variant_holding<T, U>::value;

//=================================================================================================

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

//=================================================================================================

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

//=================================================================================================

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

} // namespace ptm
